#include <iostream>
#include <fstream>
#include <direct.h>
#include <windows.h>
#include <vector>
#include <sstream>
using namespace std;

//variables globales
ifstream data;
char temp;
string tword;
string line;
string ttype;

//creacion de disco
void disk_creation(){
	int dspace;
	cout << "Ingrese el tamaño del disco en megabytes: ";
	cin >> dspace;
	mkdir("Disco");
	SetCurrentDirectory("Disco"); 
	ofstream meta("meta.txt");
	meta << dspace << "-MB";
	meta.close();
	SetCurrentDirectory("..");
}
	
//crea el esquema con las relaciones, los atributos y sus tipo de datos
void add_scheme(){
	string rel;
	bool it0=true;
	ifstream schemex("scheme.txt");
	if(schemex.peek()!=EOF){
		cout<<'\n';
	}
	schemex.close();
	ofstream scheme("scheme.txt",ios::app);
	while(::data.get(temp)){
		if(it0==true){
			cout<<"Ingrese el nombre de la relacion: " ;
			cin>>rel;
			line+= rel + "#";
			it0=false;
		}
		if(temp==','){
			cout << "Ingresa el tipo de dato del atributo " << tword << " : ";
			cin >> ttype;
			line+= tword + "#" + ttype + "#";
			tword.clear();
		}
		else if(temp=='\n'){
			cout << "Ingresa el tipo de dato del atributo " << tword << " : ";
			cin >> ttype;
			line+= tword + "#" + ttype + "\n";
			scheme << line;
			line.clear();
			tword.clear();
			ofstream res (rel + ".txt");
			while(::data.get(temp)){
				if(temp==','){
					if(tword.size()==0){
						line+="0#";
					}
					else{
						line+=tword+"#";
						tword.clear();
					}
					if(::data.peek()=='"'){
						::data.get(temp);
						tword += temp;
						while(::data.get(temp)){
							tword += temp;
							//cout<<tword<<endl;
							if(temp=='"'){
								break;
							}
						}
					}
				}
				else if(temp=='\n'){
					if(tword.size()==0){
						line+="0\n";
					}
					else{
						line+=tword+'\n';
					}
					res << line;
					line.clear();
					tword.clear();
				}
				else{
					tword += temp;
				}
				if(temp=='\n' && ::data.peek()=='\n'){
					it0=true;
					::data.get(temp);
					//cout<<tword;
					tword.clear();
					res.close();
					break;
				}
				else if(::data.peek()==EOF){
					line+=tword+'\n';
					res << line;
					tword.clear();
					res.close();
					break;
				}
			}
		}
		else{
			tword += temp;
		}
	}
	scheme.close();
}

//funcion encargada de reconocer las consultas "&SELECT * FROM tablax #"
//y "&SELECT * FROM tablax WHERE xatrib==1 #"
//y "&SELECT * FROM tablax WHERE xatrib==1 | tablay #" 
void sql_1query(string& query){
	vector<string> tokens;
	
	//Separador
	istringstream tokenstream(query);
	while(getline(tokenstream,tword,' ')){
		tokens.push_back(tword);
	}
	tword.clear();
	
	//Analizador léxico y sintáctico para la consulta SQL
	if(tokens.size()>=3&&tokens[0]=="&SELECT"&&tokens[1]=="*"&&tokens[2]=="FROM"){
		ifstream scheme("scheme.txt");
		ifstream rel;
		//comprobar si la relacion consultada existe y seleccionarla
		while(scheme.get(temp)){
			if(temp=='#'){
				if(tword==tokens[3]){
					rel.open(tokens[3] + ".txt",ios::binary);
					tword.clear();
					break;
				}
				tword.clear();
				while(scheme.get(temp)){
					if(temp=='\n'){
						scheme.get(temp);
						break;
					}
				}
			}
			tword+=temp;
		}
		//consulta de lectura
		if(tokens.size() >= 5 && rel.is_open() && tokens[4]=="#"){
			cout<<"Consulta SQL valida\n";
			//leer los datos de la relacion
			bool it1=true;
			int it2=0;
			//lee los atributos de la relacion desde el esquema
			while(scheme.get(temp)){
				if(temp=='#'){
					cout<<" | ";
					it2+=3;
					while(scheme.get(temp)){
						if(temp=='#'){
							break;
						}
						if(temp=='\n'){
							it1=false;
							break;
						}
					}
				}
				else{
					cout<<temp;
					it2++;
				}
				if(it1==false){
					break;
				}
			}
			scheme.close();
			cout<<endl;
			for(; it2>0; it2--){
				cout<<'_';
			}
			cout<<endl;
			//leer los datos almacenados en la relacion
			while(rel.get(temp)){
				if(temp=='#'){
					cout<< " | ";
				}
				else{
					cout<<temp;
				}
			}
			rel.close();
		}
		//consulta SQL con condicional
		else if(tokens.size()>=6 && rel.is_open()&&tokens[4]=="WHERE" && tokens[5].size()>=3){
			//separar las estructuras tipo a>5, b<=15, c==4 
			string tempto = tokens[5];
			string idatrib;
			string valcomp;
			string datx;
			bool it1=true;
			for(int i=0; i<tempto.size(); i++){
				if(tempto[i]=='>'||tempto[i]=='<'||tempto[i]=='='){
					for(;i<tempto.size();i++){
						if(tempto[i]=='>'||tempto[i]=='<'||tempto[i]=='='){
							valcomp+=tempto[i];
						}
						if(valcomp.size()==2){
							i++;
							break;
						}
						else if(tempto[i+1]!='='){
							i++;
							break;
						}
					}
					for(;i<tempto.size();i++){
						datx+=tempto[i];
					}
					break;
				}
				else{
					idatrib+=tempto[i];
				}
			}
			//comprobaciones adicionales
			//comprobacion del comparador para ver si es una operacion logica reconocida
			if(valcomp=="<"||valcomp==">"||valcomp=="<="||valcomp==">="||valcomp=="=="){
				cout<<"comparador valido\n";
			}
			else{
				it1=false;
				cout<<"comparador no es valido\n";
			}
			if(it1==true){
				//comprobar si el atributo consultado existe en la relacion consultada usando el esquema
				int it2=0;
				while(scheme.get(temp)){
					if(temp=='#'){
						if(idatrib==tword){
							cout<<"El atributo de la relacion existe\n";
							tword.clear();
							break;
						}
						else{
							tword.clear();
							it2++;
						}
						while(scheme.get(temp)){
							if(temp=='#'){
								break;
							}
							if(temp=='\n'){
								it1=false;
								break;
							}
						}
					}
					else{
						tword+=temp;
					}
					if(it1==false){
						cout<<"El atributo de la relacion no existe\n";
						break;
					}
				}
				//comprobar si el tipo del atributo consultado concuerda con lo que esta en el esquema
				//(de momento solo acepta float64, int64 u object para las consultas)
				if(it1==true){
					string valtype;
					while(scheme.get(temp)){
						if(temp=='#'||temp=='\n'){
							if("int64"==tword||"float64"==tword||("object"==tword&&valcomp=="==")){
								valtype=tword;
								cout<<"El tipo del atributo concuerda\n";
							}
							else{
								it1=false;
								cout<<"El tipo del atributo no concuerda\n";
							}
							tword.clear();
							temp=' ';
							break;
						}
						tword+=temp;
					}
					if(it1==true&&(tokens.size()==7||tokens.size()==9)){
						//lectura de datos de la relacion
						if(tokens[6]=="#"){
							cout<<endl;
							//lectura de atributos de la relacion
							scheme.seekg(0,ios::beg);
							while(scheme.get(temp)){
								if(temp=='#'){
									if(tword==tokens[3]){
										tword.clear();
										while(scheme.get(temp)){
											if(temp=='#'){
												while(scheme.get(temp)){
													if(temp=='#'){
														cout<<" | ";
														break;
													}
													else if(temp=='\n'){
														break;
													}
												}
											}
											else{
												cout<<temp;
											}
											if(temp=='\n'){
												break;
											}
										}
										break;
									}
									else{
										tword.clear();
										while(scheme.get(temp)){
											if(temp=='\n'){
												scheme.get(temp);
												break;
											}
										}
									}
								}
								tword+=temp;
							}
							cout<<endl;
							scheme.close();
							//bucle para extraer todas las lineas del atributo de la relacion consultada
							int it4=it2;
							int it5=0;
							streampos pos_y;
							while(rel.peek()!=EOF){
								//lectura del atributo a comparar
								it4=it2;
								it5=0;
								pos_y=rel.tellg();
								if(it4==0){
									while(rel.get(temp)){
										it5++;
										if(temp=='#'){
											break;
										}
										else if(temp=='\n'){
											rel.unget();
											break;
										}
										tword+=temp;
									} 
								}
								else{
									while(rel.get(temp)){
										it5++;
										if(it4==0){
											while(rel.get(temp)){
												it5++;
												if(temp=='#'){
													break;
												}
												else if(temp=='\n'){
													rel.unget();
													break;
												}
												tword+=temp;
											} 
											break;
										}
										if(rel.peek()=='#'){
											it4--;
											tword.clear();
										}
									}
								}
								it5--;
								//comprobacino de las operaciones logicas (comparar dos valores)
								bool it3=false;
								if(valcomp=="=="&&tword==datx){
									it3=true;
								}
								if(valcomp=="<"){
									if(valtype=="int64"){int xval=stoi(tword);int yval=stoi(datx);
										if(xval<yval){it3=true;}
									}
									else if(valtype=="float64"){float xval=stof(tword);float yval=stof(datx);
										if(xval<yval){it3=true;}
									}
								}
								else if(valcomp==">"){
									if(valtype=="int64"){int xval=stoi(tword);int yval=stoi(datx);
										if(xval>yval){it3=true;}
									}
									else if(valtype=="float64"){float xval=stof(tword);float yval=stof(datx);
										if(xval>yval){it3=true;}
									}
								}
								else if(valcomp=="<="){
									if(valtype=="int64"){int xval=stoi(tword);int yval=stoi(datx);
										if(xval<=yval){it3=true;}
									}
									else if(valtype=="float64"){float xval=stof(tword);float yval=stof(datx);
										if(xval<=yval){it3=true;}
									}
								}
								else if(valcomp==">="){
									if(valtype=="int64"){int xval=stoi(tword);int yval=stoi(datx);
										if(xval>=yval){it3=true;}
									}
									else if(valtype=="float64"){float xval=stof(tword);float yval=stof(datx);
										if(xval>=yval){it3=true;}
									}
								}
								//solo si la operacion logica es cierta
								if(it3==true){
									//regresa la lectura al inicio de la linea
									rel.seekg(pos_y);
									//consumir y guardar la linea totalmente
									while(rel.get(temp)){
										if(temp=='\n'){
											cout<<'\n';
											tword.clear();
											break;
										}
										else if(temp=='#'){
											cout<<" | ";
										}
										else{
											cout<<temp;
										}
									}
								}
								else{
									while(rel.get(temp)){
										if(temp=='\n'){
											break;
										}
									}
								}
								if(temp=='\n'&&rel.peek()=='\n'){
									break;
								}
							}
							rel.close();
						}
						//lectura y guardado de la relacion
						else if(tokens[6]=="|" && tokens[8]=="#"){
							cout<<endl;
							//lectura y guardado de atributos de la relacion
							ofstream nrel_sch("scheme.txt",ios::app);
							nrel_sch<<tokens[7]<<'#';
							scheme.seekg(0,ios::beg);
							while(scheme.get(temp)){
								if(temp=='#'){
									if(tword==tokens[3]){
										tword.clear();
										while(scheme.get(temp)){
											if(temp=='#'){
												nrel_sch<<temp;
												while(scheme.get(temp)){
													nrel_sch<<temp;
													if(temp=='#'){
														cout<<" | ";
														break;
													}
													else if(temp=='\n'){
														break;
													}
												}
											}
											else{
												nrel_sch<<temp;
												cout<<temp;
											}
											if(temp=='\n'){
												break;
											}
										}
										break;
									}
									else{
										tword.clear();
										while(scheme.get(temp)){
											if(temp=='\n'){
												scheme.get(temp);
												break;
											}
										}
									}
								}
								tword+=temp;
							}
							cout<<endl;
							nrel_sch.close();
							scheme.close();
							//bucle para extraer todas las lineas del atributo de la relacion consultada
							ofstream nrel(tokens[7]+".txt");
							int it4=it2;
							int it5=0;
							streampos pos_y;
							while(rel.peek()!=EOF){
								//lectura del atributo a comparar
								it4=it2;
								it5=0;
								pos_y=rel.tellg();
								if(it4==0){
									while(rel.get(temp)){
										it5++;
										if(temp=='#'){
											break;
										}
										else if(temp=='\n'){
											rel.unget();
											break;
										}
										tword+=temp;
									} 
								}
								else{
									while(rel.get(temp)){
										it5++;
										if(it4==0){
											while(rel.get(temp)){
												it5++;
												if(temp=='#'){
													break;
												}
												else if(temp=='\n'){
													rel.unget();
													break;
												}
												tword+=temp;
											} 
											break;
										}
										if(rel.peek()=='#'){
											it4--;
											tword.clear();
										}
									}
								}
								it5--;
								//realizacion de las operaciones logicas
								bool it3=false;
								if(valcomp=="=="&&tword==datx){
									it3=true;
								}
								if(valcomp=="<"){
									if(valtype=="int64"){int xval=stoi(tword);int yval=stoi(datx);
									if(xval<yval){it3=true;}
									}
									else if(valtype=="float64"){float xval=stof(tword);float yval=stof(datx);
									if(xval<yval){it3=true;}
									}
								}
								else if(valcomp==">"){
									if(valtype=="int64"){int xval=stoi(tword);int yval=stoi(datx);
									if(xval>yval){it3=true;}
									}
									else if(valtype=="float64"){float xval=stof(tword);float yval=stof(datx);
									if(xval>yval){it3=true;}
									}
								}
								else if(valcomp=="<="){
									if(valtype=="int64"){int xval=stoi(tword);int yval=stoi(datx);
									if(xval<=yval){it3=true;}
									}
									else if(valtype=="float64"){float xval=stof(tword);float yval=stof(datx);
									if(xval<=yval){it3=true;}
									}
								}
								else if(valcomp==">="){
									if(valtype=="int64"){int xval=stoi(tword);int yval=stoi(datx);
									if(xval>=yval){it3=true;}
									}
									else if(valtype=="float64"){float xval=stof(tword);float yval=stof(datx);
									if(xval>=yval){it3=true;}
									}
								}
								tword.clear();
								//solo si la operacion logica es cierta
								if(it3==true){
									//regresa la lectura al inicio de la linea
									rel.seekg(pos_y);
									//consumir y guardar la linea totalmente
									while(rel.get(temp)){
										nrel<<temp;
										if(temp=='\n'){
											cout<<'\n';
											break;
										}
										else if(temp=='#'){
											cout<<" | ";
										}
										else{
											cout<<temp;
										}
									}
								}
								else{
									while(rel.get(temp)){
										if(temp=='\n'){
											break;
										}
									}
								}
								if(temp=='\n'&&rel.peek()=='\n'){
									break;
								}
							}
							nrel.close();
							rel.close();
						}
						else{
							cout<<"Consulta SQL no valida\n";
							scheme.close();
						}
					}
				}
			}
		}
		else{
			cout<<"Consulta SQL no valida\n";
			scheme.close();
		}
	}
	else{
		cout<<"Consulta SQL no valida\n";
	}
}
//funcion para anadir registros
void add_regis(string& relx){
	ifstream scheme("scheme.txt");
	ofstream rel;
	string datx;
	string atribx;
	string typex;
	int itx=0;
	while(scheme.get(temp)){
		if(temp=='#'){
			if(tword==relx){
				rel.open(relx + ".txt",ios::binary|ios::app);
				tword.clear();
				break;
			}
			tword.clear();
			while(scheme.get(temp)){
				if(temp=='\n'){
					scheme.get(temp);
					break;
				}
			}
		}
		tword+=temp;
	}
	if(rel.is_open()){
		while(scheme.get(temp)){
			itx=0;
			if(temp=='#'){
				cout<<"Inserte el dato para el atributo "<<tword<<" : ";
				cin.ignore();
				getline(cin,datx);
				if(itx==0){
					rel<<datx<<"#";
				}
				else{
					rel<<datx;
				}
				tword.clear();
				while(scheme.get(temp)){
					if(temp=='#'){
						break;
					}
					else if(temp=='\n'){
						itx=1;
						break;
					}
				}
			}
			else if(temp=='\n'){
				break;
			}
			else{
				tword+=temp;
			}
		}
	}
	scheme.close();
}
	
int main(int argc, char *argv[]) {
	cout << " MENU PRINCIPAL\n";
	int it_m1;
	do{
		cout << "1. Crear Disco\n";
		cout << "2. Agregar Esquema\n";
		cout << "3. Hacer una consulta\n";
		cout << "4. Anadir registro\n";
		cout << "5. Salir\n";
		cout << "Escribe la opcion escogida: ";
		cin >> it_m1;
		if(it_m1==1){
			system("CLS");
			cout << "SUBMENU (DISCO)\n";
			disk_creation();
			system("pause");
			system("CLS");
		}
		else if(it_m1==2){
			system("CLS");
			cout << "SUBMENU (ANADIR ESQUEMA)\n";
			string db_name;
			cout << "Escriba el nombre de su base de datos (.csv): ";
			cin >> db_name;
			::data.open(db_name);
			if(SetCurrentDirectory("Disco")){
				add_scheme();
				SetCurrentDirectory("..");
			}
			::data.close();
			system("pause");
			system("CLS");
		}
		else if (it_m1==3 && SetCurrentDirectory("Disco")){
			system("CLS");
			cout << "SUBMENU (CONSULTA \"SELECT\")\n";
			string query;
			cout<<"Escriba su consulta: ";
			cin.ignore();
			getline(cin,query);
			sql_1query(query);
			SetCurrentDirectory("..");
			system("pause");
			system("CLS");
		}
		else if(it_m1==4 && SetCurrentDirectory("Disco")){
			system("CLS");
			cout << "SUBMENU (ANADIR REGISTRO)\n";
			string relx;
			cout << "Ingrese la relacion que quiere modificar: ";
			cin >>relx;
			add_regis(relx);
			SetCurrentDirectory("..");
			system("pause");
			system("CLS");
		}
		else if (it_m1==5){
			cout << "\nPrograma finalizado\n";
			system("pause");
			break;
		}
		else{
			cout << "\nOpcion Incorrecta\n";
			system("pause");
			system("CLS");
		}
	} while(true);
	return 0;
}
//COSAS A MEJORAR/IMPLEMENTAR
//mejorar add_regis (necesita comprobar los tipos de datos)
//acoplar con la capacidad del disco
//considerar comillas para los nombres de los atributos
//agregar busquedas con comillas tanto para los nombres
//dividir add_scheme en dos partes add_scheme y charge_data
