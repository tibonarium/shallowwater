

#include <math.h>
#include <time.h>
#include <stdio.h>

#include <algorithm>
#include <iostream>
#include <fstream>
using namespace std;


//количество запоминаемых значащих цифр
#define NDEC 10

//#define MIN_X 0.01


//**************************************************************************
//	{Eta} = 5000 - {H} - {B}
//**************************************************************************

int main(int argn, char **argv) 
{ 

	//**************************************************************************
	//	Инициализация переменных
	//**************************************************************************

	
	int Fit;
	
	double g = 9.8;
	int i,j;

	double T0 = 74.5; // Движение береговой линии нужно выводить до момента 360s
	double alpha = 0.2; //hx=0.025 1)alpha=0.1 2)alpha=0.1
	double beta = 0.1;	//hx=0.025 1)beta=0.2  2)beta=0.1
	
	// НУЖНО в ручную вводить шаг сетки
	double hx = 0.025;	//0.05	0.025
	int Nx;	

	double MAX_H = 100/30;

	double dT = (beta*hx)/sqrt(g*MAX_H);

	double epsilon = hx/30;

	
	double step_export_graphic = 5; // 
	double T_export_graphic_start = 5; // 
	double T_export_graphic_end = 75;
	

	double T_export_data = 3240; // 3240 s

	//**************************************************************************
	// Подсчет числа строчек, чтобы узнать каких размеров создавать массив	
	//**************************************************************************
	ifstream pFile1;
	pFile1.open("C_InitialConditions_tsunami_runup_Another.dat");

	char c;
	int Number_Initial = 1;
	while(pFile1.get(c))
	{
		if(c == '\n')
		{
			Number_Initial += 1;
		}
	}

	pFile1.close();

	//**************************************************************************
	//Создание массивов необходимых размеров и чтение из файла	
	//**************************************************************************
	
	ifstream p2File("C_InitialConditions_tsunami_runup_Another.dat");
		
	cout << Number_Initial << endl;
	cout << dT << endl;

	//cin >> Fit;

	Nx = Number_Initial - 1;

	double* X = new double[Nx];
	
	double* H = new double[Nx];
	double* B = new double[Nx];
	double* U = new double[Nx];
	
	
	double* Force = new double[Nx];           
	double* tau = new double[Nx];

	double* Ht = new double[Nx];
	double* Ut = new double[Nx];

	
	for (i=0; i<Number_Initial; i++)
	{		
		p2File >> X[i];
		p2File >> H[i];
		p2File >> B[i];
		p2File >> U[i];		
	}	

	p2File.close();

	

	//**************************************************************************
	//	Инициализация массивов и переменных
	//**************************************************************************
	

	
	// Промежуточные переменные для расчетов
	
	

	double H2x2,H2x1;
	double U2x2,U2x1;
	double B2x2,B2x1;

	double tau2x2,tau2x1;
	double Force2x2,Force2x1;
	
	double J2x2,J2x1;
	double PT2x2,PT2x1;
		


	double X_pp, X_p;
	double dT_p;
	double U_shore;

	int i_shore;	// переменная для хранения индекса побережья

	// T_current фиксирует время задачи. Оно берется из dat файла с данными
	double T_current = 0.0;

	// К величине T_circle прибавляется переменный шаг по времени dT
	double T_circle = 0.0;
	
	// указываем не конечную точку расчета,промежуток в течении которого ведется расчет
	double T_add = T0;

	

	// Эта переменная используется при выводе процентов в консольную строчку
	int dN = 1;

	// Указатель, который понадобится при экспорте одного временного слоя
	FILE* f_id_tecplot;
	// Указатель, который используем для обозначения файла с несколькими временными слоями
	FILE* f_id_1D_FLOW;

	// указатель для файла, в котором записываем положение береговой линии
	FILE* f_id_Shore;

	//TEMP
	FILE* f_id_Errors;
	
	// Строка и указатель нужны для конструирования имени файла с цифрами
	string Buffer_Name_Tecplot;
	char* p_number;

	char buffer_name[1000];
	int N_smb;

	int Percentage_N;
	
	int stop = 0;



	//**************************************************************************
	//	Начальные данные записываем в dat файл Текплота
	//**************************************************************************


	FILE* pFile;
	pFile = fopen("Initial_conditions.dat","w");

	// TITLE="TSUNAMI RUNUP ONTO A PLANE BEACH"
	N_smb = sprintf( buffer_name,"TITLE=\"TSUNAMI RUNUP ONTO A PLANE BEACH\" \n");
	fwrite(&buffer_name, sizeof(char), N_smb, pFile);
		
	//VARIABLES = X,H,U,B 
	N_smb = sprintf( buffer_name,"VARIABLES = X,H,U,B \n");
	fwrite(&buffer_name, sizeof(char), N_smb, pFile);

	// ZONE T="Time 0.0", I=%d, DATAPACKING=POINT
	N_smb = sprintf( buffer_name,"ZONE T=\"Time 0.0\", I=%d, DATAPACKING=POINT\n",Nx);
	fwrite(&buffer_name, sizeof(char), N_smb, pFile);
	
	for (i=0; i<Nx; i++)
	{
		N_smb = sprintf( buffer_name,"%f\t%f\t%f\t%f\n", X[i], H[i], U[i], B[i]);
		fwrite(&buffer_name, sizeof(char), N_smb, pFile);
	}

	fclose(pFile);


	//**************************************************************************
	//	
	//**************************************************************************


	MAX_H = H[1];
	for (i=0; i<Nx; i++)
	{
		if (MAX_H < H[i])	MAX_H = H[i];			
	}

	
	for (i=0; i<Nx; i++)
	{
		if (H[i]>epsilon)
		{			
			tau[i] = alpha*hx/sqrt(g*H[i]);
		}
		else
		{			
			tau[i] = 0.0;
		}

		Force[i] = 0.0;

		
	}

	//**************************************************************************
	//	Основной цикл по времени while (T_circle < T_add)
	//**************************************************************************

	//TEMP
	int number_circle = 1;

	//TEMP
	//int number_for_data_export = 1;

	//TEMP
	double Ksi = 0.0;

	// TEMP
	double Temp_variable;


	// указатель на файл с граничными данным
	ifstream f_id_Border("C_BorderConditions_tsunami_runup_Another.dat");

	double T_input;
	double H_input;
	double U_input;

	double T_next_input;
	double H_next_input;
	double U_next_input;

	
	f_id_Border >> T_input;
	f_id_Border >> H_input;
	f_id_Border >> U_input;

	f_id_Border >> T_next_input;
	f_id_Border >> H_next_input;
	f_id_Border >> U_next_input;

	// 1D_Shore
	Buffer_Name_Tecplot += "1D_Shore";
							
	// alpha
	Buffer_Name_Tecplot += " alpha=";
		p_number = gcvt(alpha,NDEC,buffer_name);
	Buffer_Name_Tecplot += p_number;
				
	// beta
	Buffer_Name_Tecplot += " beta=";
		p_number = gcvt(beta,NDEC,buffer_name);
	Buffer_Name_Tecplot += p_number;
				
	// hx
	Buffer_Name_Tecplot += " hx=";
		p_number = gcvt(hx,NDEC,buffer_name);
	Buffer_Name_Tecplot += p_number;
				
	Buffer_Name_Tecplot += ".dat";
												
	f_id_Shore = fopen(Buffer_Name_Tecplot.c_str(),"w");

	Buffer_Name_Tecplot.erase();


	// К величине T_circle прибавляется переменный шаг по времени dT
	while (T_circle < T_add)
	{
		
		MAX_H = 0.0;
		for (i=0; i<Nx; i++)
		{
			if (MAX_H < H[i])	MAX_H = H[i];			
		}
		
		dT = (beta*hx)/sqrt(g*MAX_H);
		
		

		T_circle += dT;
		number_circle += 1;
		
	
		// Если в процессее расчета Ht<0, то stop = 1
		if (stop==0)
		{       
			for (i=1; i<(Nx-1); i++)
			{
				
				if (i==1)
				{
					B2x1 = 0.5*(B[i] + B[i-1]);
					H2x1 = 0.5*(H[i] + H[i-1]);

					Force2x1 = 0.5*( Force[i-1] + Force[i] );

					// TEMP
					//**************************************************************************
					//	Согласование условий H<epsilon => u=0, tau=0
					//**************************************************************************
					if (H2x1>epsilon)
					{				
						U2x1 = 0.5*(U[i] + U[i-1]);
						tau2x1 = 0.5*(tau[i] + tau[i-1]);
					}
					else
					{
						U2x1 = 0.0;
						tau2x1 = 0.0;		
						
						//H2x1 = 0.0;
					}
					
					J2x1 = H2x1*U2x1 - tau2x1*( ( H[i]*U[i]*U[i] - H[i-1]*U[i-1]*U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i] - B[i-1] )/hx - H2x1*Force2x1 );

					PT2x1 = tau2x1*U2x1*( H2x1*U2x1*( U[i] - U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i]-B[i-1] )/hx - H2x1*Force2x1 ) + g*tau2x1*( U2x1*( 0.5*H[i]*H[i]-0.5*H[i-1]*H[i-1] )/hx + H2x1*H2x1*( U[i]-U[i-1] )/hx );
				
				

				}

				// Достаточно для каждого i вычислять 2x2

				B2x2 = 0.5*(B[i+1] + B[i]);				
				//
				H2x2 = 0.5*(H[i+1] + H[i]);

				// TEMP
				if (H2x2>epsilon)
				{					
					U2x2 = 0.5*(U[i+1] + U[i]);
					tau2x2 = 0.5*(tau[i+1] + tau[i]);
				}
				else
				{					
					U2x2 = 0.0;
					tau2x2 = 0.0;

					//H2x2 = 0.0;
				}
						

				//
				//U2x2 = 0.5*(U[i+1] + U[i]);
				//U2x1 = 0.5*(U[i] + U[i-1]);
				//
				//tau2x2 = 0.5*(tau[i+1] + tau[i]);
				//tau2x1 = 0.5*(tau[i] + tau[i-1]);
				//
				Force2x2 = 0.5*( Force[i] + Force[i+1] );
				
				
				//J2x2
				J2x2 = H2x2*U2x2 - tau2x2*( ( H[i+1]*U[i+1]*U[i+1] - H[i]*U[i]*U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1] - B[i] )/hx - H2x2*Force2x2 );
				
				//J2x1
				//J2x1 = H2x1*U2x1 - tau2x1*( ( H[i]*U[i]*U[i] - H[i-1]*U[i-1]*U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i] - B[i-1] )/hx - H2x1*Force2x1 );
				
										
				//Ht
				Ht[i] = H[i] - (dT/hx)*( J2x2 - J2x1 ); 

				
				
				//PT2x2
				PT2x2 = tau2x2*U2x2*( H2x2*U2x2*( U[i+1] - U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1]-B[i] )/hx - H2x2*Force2x2 ) + g*tau2x2*( U2x2*( 0.5*H[i+1]*H[i+1]-0.5*H[i]*H[i] )/hx + H2x2*H2x2*( U[i+1]-U[i] )/hx );
				
				//PT2x1
				//PT2x1 = tau2x1*U2x1*( H2x1*U2x1*( U[i] - U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i]-B[i-1] )/hx - H2x1*Force2x1 ) + g*tau2x1*( U2x1*( 0.5*H[i]*H[i]-0.5*H[i-1]*H[i-1] )/hx + H2x1*H2x1*( U[i]-U[i-1] )/hx );
				
						
				if (Ht[i] < 0.0)
				{
					stop = 1;
					

					// TEMP
					cout << "Very bad\nNumber_circle=" << number_circle << "\ni= " << i << "\nX= " << X[i] << "\nHt[i]= " << Ht[i] << "\nH[i]= " << H[i] << endl;
					// i=24 23*5 = 115 m
					// dH = H[i] - Ht[i] < H[t]
					cout << "number_circle= " << number_circle << "\nT_circle= " << T_circle << endl;
					cout << "\nLook at U" << endl;
					cout << "U[i-1]= " << U[i-1] << "\nU[i]= " << U[i] <<"\nU[i+1]= " << U[i+1] << endl;
					cout << "\nLook at H" << endl;
					cout << "H[i-1]= " << H[i-1] << "\nH[i]= " << H[i] <<"\nH[i+1]= " << H[i+1] << endl;
					
					cout << "\nMore Info" << endl;
					cout << ( J2x2 - J2x1 ) << "\n" << J2x2 << "\n" << J2x1 << endl;

					Temp_variable = ( H[i+1] - H[i] )/hx;
					cout << Temp_variable << endl;

					Temp_variable = ( B[i+1] - B[i] )/hx;
					cout << Temp_variable << endl;

					cout << "\t++++++++++++++Next++++++++++++++++" << endl;
					cout << H2x2*U2x2 << "\n" << H2x1*U2x1 << "\n" << tau2x2 << "\n" << tau2x1 << endl;
					
					cout << "Add" << endl; 
					//cout << H2x2*Force2x2 << "\n" << H2x1*Force2x1 << endl;	//	0	0	

					Temp_variable = g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx;	//	8e-6
					cout << Temp_variable << endl;

					Temp_variable = g*H2x2*( B[i+1] - B[i] )/hx;				//	-8e5
					cout << Temp_variable << endl;

					Temp_variable = g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx;	//	-3e7
					cout << Temp_variable << endl;

					Temp_variable = g*H2x1*( B[i] - B[i-1] )/hx;				//	-1e5
					cout << Temp_variable << endl;

					// TEMP
					f_id_Errors = fopen("Errors_collect.txt","w");

					// "Very bad\n"
					N_smb = sprintf( buffer_name,"Very bad\n");
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_Errors);

					// "number_circle = (...)\n"
					N_smb = sprintf( buffer_name,"number_circle = %d\n", number_circle);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_Errors);

					// "i = (...)\n"
					N_smb = sprintf( buffer_name,"i = %d\n", i);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_Errors);

					// "X[i] = (...)\n"
					N_smb = sprintf( buffer_name,"X[i] = %f\n", X[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_Errors);

					// "T_circle = (...)\n"
					N_smb = sprintf( buffer_name,"T_circle = %f\n", T_circle);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_Errors);
					
					// "U[i-1] = (...)\t U[i] = (...)\t U[i+1] = (...)\n"
					N_smb = sprintf( buffer_name,"U[i-1] = %f\t U[i] = %f\t U[i+1] = %f\n", U[i-1], U[i], U[i+1]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_Errors);

					// "H[i-1] = (...)\t H[i] = (...)\t H[i+1] = (...)\n"
					N_smb = sprintf( buffer_name,"H[i-1] = %f\t H[i] = %f\t H[i+1] = %f\n", H[i-1], H[i], H[i+1]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_Errors);

					// "Ht[i] = (...)\n"
					N_smb = sprintf( buffer_name,"Ht[i] = %f\n", Ht[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_Errors);

					fclose(f_id_Errors);

					//cin >> Fit;

				
				}

							
				
				if (Ht[i] > epsilon)
				{
					
					//Ut[i] = ( H[i]*U[i] + (dT/hx)*( PT2x2-PT2x1 ) - (dT/hx)*( U2x2*J2x2-U2x1*J2x1 ) - g*(dT/hx)*( 0.5*H2x2*H2x2-0.5*H2x1*H2x1 ) + dT*( H[i] - tau[i]*( H2x2*U2x2-H2x1*U2x1 )/hx )*( Force[i] - g*( B2x2-B2x1 )/hx ) )/Ht[i];
					
					// H -> 0.5*(H2x2+H2x1)
					Ut[i] = ( H[i]*U[i] + (dT/hx)*( PT2x2-PT2x1 ) - (dT/hx)*( U2x2*J2x2-U2x1*J2x1 ) - g*(dT/hx)*( 0.5*H2x2*H2x2-0.5*H2x1*H2x1 ) + dT*( 0.5*(H2x2+H2x1) - tau[i]*( H2x2*U2x2-H2x1*U2x1 )/hx )*( Force[i] - g*( B2x2-B2x1 )/hx ) )/Ht[i];
					
										
				}
				else
				{
					
					Ut[i] = 0.0;

					/*
					 Stabilized residual distribution for shallow water simulations
					 M.Ricchiuto, A. Bollermann
					 Journal of Computational Physics
					 Volume 228 Issue 4, March 2009

					http://portal.acm.org/citation.cfm?id=1486474
					///
					*/
			

				}

				
				// приравниваем величины с точки 2x2 к точке 2x1
				B2x1 = B2x2;
				H2x1 = H2x2;

				U2x1 = U2x2;
				tau2x1 = tau2x2;
				Force2x1 = Force2x2;
	
				J2x1 = J2x2;
				PT2x1 = PT2x2;
		
			
			}
			//конец for (i=1; i<=Nx; i++)

			//**************************************************************************
			//	Обработка граничных условий
			//**************************************************************************

			if (T_circle >= T_next_input)
			{
				T_input = T_next_input;
				H_input = H_next_input;
				U_input = U_next_input;
				
				f_id_Border >> T_next_input;
				f_id_Border >> H_next_input;
				f_id_Border >> U_next_input;
			}
		
			//Граничные точки
			Ht[0] = -Ht[1] +2*(H_input + (T_circle-T_input)*(H_next_input - H_input)/(T_next_input - T_input));
			Ut[0] = -Ut[1] +2*(U_input + (T_circle-T_input)*(U_next_input - U_input)/(T_next_input - T_input));

			Ht[Nx-1] = Ht[Nx-2];
			Ut[Nx-1] = Ut[Nx-2];

			for (i=0; i<Nx; i++)
			{
				H[i] = Ht[i];
				//U[i] = Ut[i];

				if (H[i]>epsilon)
				{					
					tau[i] = alpha*hx/sqrt(g*H[i]);

					U[i] = Ut[i];
				}
				else
				{					
					//tau[i] = alpha*hx/(sqrt(g*H[i])+50);
					tau[i] = 0.0;

					U[i] = 0.0;
					
				}

			}

			///////////////////////////////////////   
		

			//**************************************************************************
			//	Ищем координату точки разделения жидкости и сухого дна	
			//**************************************************************************
			//	

			for (i=1; i<(Nx-1); i++)
			{
				// Это условие предполагает, что сухое дно
				// располагается СПРАВА
				if (H[i]<=1.05*epsilon && H[i-1]>1.05*epsilon)
				{
					i_shore = i;
				}			


			}

			
		

			//(T_circle) \t (X[i_shore]) \n
			N_smb = sprintf( buffer_name,"%f\t%f\n", T_circle, X[i_shore]);
			fwrite(&buffer_name, sizeof(char), N_smb, f_id_Shore);
			
			
				
        
			//**************************************************************************
			//	Вывот процентного соотношения выволненнных расчетов в консольную строку		
			//**************************************************************************
			
			Percentage_N = int(100*T_circle/T_add);
			if (Percentage_N >= dN)
			{	
				cout << Percentage_N << "%\n";
				dN += 1;
			}

			//**************************************************************************
			//	Вывод данных в файлы	
			//**************************************************************************
			
	

			if (T_export_graphic_start <= T_circle && T_circle < T_export_graphic_end)
			//if (number_circle == 2 || number_circle == 3)
			{
				// Export to TecPlot  

				// Начинаем конструировать имя файла
				// "Tecplot T0=(T_export_graphic) alpha=(...) beta=(...) hx=(...).dat"
				
				
				// T0
				Buffer_Name_Tecplot += "Tecplot T0=";
					p_number = gcvt(T_export_graphic_start+T_current,NDEC,buffer_name);
				Buffer_Name_Tecplot += p_number;
				
				// alpha
				Buffer_Name_Tecplot += " alpha=";
					p_number = gcvt(alpha,NDEC,buffer_name);
				Buffer_Name_Tecplot += p_number;
				
				// beta
				Buffer_Name_Tecplot += " beta=";
					p_number = gcvt(beta,NDEC,buffer_name);
				Buffer_Name_Tecplot += p_number;
				
				// hx
				Buffer_Name_Tecplot += " hx=";
					p_number = gcvt(hx,NDEC,buffer_name);
				Buffer_Name_Tecplot += p_number;
				

				/*
				//TEMP
					p_number = gcvt(number_for_data_export,NDEC,buffer_name);
				Buffer_Name_Tecplot += p_number;

				//TEMP
				number_for_data_export += 1;
				*/
				
				
				Buffer_Name_Tecplot += ".dat";
												
				f_id_tecplot = fopen(Buffer_Name_Tecplot.c_str(),"w");
				
				
				// "VARIABLES = X,H,U \n"
				
				N_smb = sprintf( buffer_name,"VARIABLES = X,Ksi,B,H,U,tau \n");
				fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
				
				// "ZONE T=(...), I=(...), DATAPACKING=POINT\n"
				
				N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", I=%d, DATAPACKING=POINT\n",T_export_graphic_start+T_current, Nx);
				fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

							
				for (i=0; i<Nx; i++)
				{
					Ksi = H[i] + B[i];
					N_smb = sprintf( buffer_name,"%f\t%f\t%f\t",X[i], Ksi, B[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
					
			
					N_smb = sprintf( buffer_name,"%f\t%f \t%f\n", H[i], U[i], tau[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

				}

				//N_smb = sprintf( buffer_name,"\n");
				//fwrite(&buffer_name, sizeof(char), N_smb, f_id_1D_FLOW);
				
				fclose(f_id_tecplot);
			
				//fclose(f_id_1D_FLOW);

				// Очищаем сроку от символов с названием файла
				Buffer_Name_Tecplot.erase();

				// 
				T_export_graphic_start += step_export_graphic;

				

			}
			// if (T_export_graphic_start <= T_circle && T_circle < T_export_graphic_end)
			
			if (T_circle<T_export_data && T_export_data<=(T_circle+dT))
			{
				// Формат бинарного файла
				// T_export_data alpha beta hx Nx
				// (i=0..Nx-1) H (i=0..Nx-1) U (i=0..Nx-1) B

				// Начинаем конструировать имя файла
				// "DataFlow Tf=(...).dat"
				
				// Tf
				Buffer_Name_Tecplot += "Tecplot Tf=";
					p_number = gcvt(T_export_data+T_current,NDEC,buffer_name);
				Buffer_Name_Tecplot += p_number;						
				
				Buffer_Name_Tecplot += ".dat";
								
				f_id_tecplot = fopen(Buffer_Name_Tecplot.c_str(),"w");

				fwrite(&T_export_data, sizeof(double), 1, f_id_tecplot);	//T_export_data
				fwrite(&alpha, sizeof(double), 1, f_id_tecplot);			//alpha
				fwrite(&beta, sizeof(double), 1, f_id_tecplot);				//beta
				fwrite(&hx, sizeof(double), 1, f_id_tecplot);				//hx
				fwrite(&Nx, sizeof(double), 1, f_id_tecplot);				//Nx

				for (i=0; i<Nx; i++)
				{
					fwrite(&H[i], sizeof(double), 1, f_id_tecplot);				//H
					fwrite(&U[i], sizeof(double), 1, f_id_tecplot);				//U
					fwrite(&B[i], sizeof(double), 1, f_id_tecplot);				//B

				}

				fclose(f_id_tecplot);

				// Очищаем сроку от символов с названием файла
				Buffer_Name_Tecplot.erase();
			}
			
		}
		// конец if (stop==0)

			
			
	}
	//конец while (T_circle < T_add)

	f_id_tecplot = fopen("Final.dat","w");
				
	// "VARIABLES = X,H,U \n"
				
	N_smb = sprintf( buffer_name,"VARIABLES = X,B,H,U \n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
				
	// "ZONE T=(...), I=(...), DATAPACKING=POINT\n"
				
	N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", I=%d, DATAPACKING=POINT\n",(T_circle+T_current), Nx);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	
				
	for (i=0; i<Nx; i++)
	{			
		N_smb = sprintf( buffer_name,"%f\t%f\t", X[i], B[i]);
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
					
		N_smb = sprintf( buffer_name,"%f\t%f \n", H[i], U[i]);
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	}

	fclose(f_id_tecplot);

	// TEMP
	f_id_Errors = fopen("Number_circles.txt","w");

	// "Additional Information\n"
	N_smb = sprintf( buffer_name,"Additional Information\n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_Errors);

	// "number_circle = (...)\n"
	N_smb = sprintf( buffer_name,"number_circle = %d\n", number_circle);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_Errors);

	fclose(f_id_Errors);

	cout << "\n\nAT LAST Number_circle= " << number_circle << endl;
     
	f_id_Border.close();

	fclose(f_id_Shore);
			
    return 0;
} 
