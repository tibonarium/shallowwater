
#include <math.h>
#include <time.h>
#include <stdio.h>

#include <algorithm>
#include <iostream>
#include <fstream>
using namespace std;


//количество запоминаемых значащих цифр
#define NDEC 10
#define MAG 50



int main(int argn, char **argv) 
{ 	

	//**************************************************************************
	//	Инициализация переменных
	//**************************************************************************

	double g = 9.8;
	int i,j;

	double T0 = 52; // 3300 s
	double alpha = 0.5; // 0.5
	double beta = 0.5; // 0.1
		
	
	double L = 1; // 1m
	int Nx = 501;
	double hx = 0.002; //m


	double epsilon = 0.01;

	// Ставится условие epsilon >= hx*|db/dx|_shore
	

	double step_export_graphic = 20; //шаг по времени с которой выводятся данные в файл
	
	// момент времени с которого начинаются выводиться данные в файл
	double T_export_graphic_start = 30;

	// конечный момент времени, до которого выводятся данные в файл
	double T_export_graphic_end = 100;

	
	double* X = new double[Nx];
	
	double* H = new double[Nx];
	double* U = new double[Nx];
	double* B = new double[Nx];
	
	double* Force = new double[Nx];           
	double* tau = new double[Nx];


	double* Ht = new double[Nx];
	double* Ut = new double[Nx];

	// Промежуточные переменные для расчетов
	
	double dT;

	double H2x2,H2x1;
	double U2x2,U2x1;
	double B2x2,B2x1;

	double tau2x2,tau2x1;
	double Force2x2,Force2x1;
	
	double J2x2,J2x1;
	double PT2x2,PT2x1;
	
	double MAX_H;

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
	
	
		
	// Строка и указатель нужны для конструирования имени файла с цифрами
	string Buffer_Name_Tecplot;
	char* p_number;

	char buffer_name[300];
	int N_smb;

	int Percentage_N;
	
	int stop = 0;

	//**************************************************************************
	//	
	//**************************************************************************

	
	//Форма дна для задачи с переодическим набеганием волны на берег
	for (j=0; j<Nx; j++)
	{
		X[j] = hx*j;

		B[j] = 0.25-5*(X[j]-0.5)*(X[j]-0.5);
		
		if (B[j] < 0)
		{
			B[j] = 0.0;
		}
		
		H[j] = 0.1 - B[j];
		
		if (H[j] < 0)
		{
			H[j] = 0.0;
		}

		U[j] = 0.0;
		Force[j] = 0.0;		
		
	}
	

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
		
	}

	//**************************************************************************
	//	Основной цикл по времени while (T_circle < T_add)
	//**************************************************************************

	//TEMP
	int number_circle = 0;

	//TEMP
	int number_for_data_export = 1;

	//TEMP
	double Ksi = 0.0;

	// TEMP
	double Temp_variable;

	//TEMP 12345
	int Fit; 

	// К величине T_circle прибавляется переменный шаг по времени dT
	while (T_circle < T_add)
	{
		MAX_H = H[1];
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
					if (H2x1>epsilon)
					{				
						U2x1 = 0.5*(U[i] + U[i-1]);
						tau2x1 = 0.5*(tau[i] + tau[i-1]);					
					}
					else
					{
						U2x1 = 0.0;
						tau2x1 = 0.0;						
					}
					
					J2x1 = H2x1*U2x1 - tau2x1*( ( H[i]*U[i]*U[i] - H[i-1]*U[i-1]*U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i] - B[i-1] )/hx - H2x1*Force2x1 );

					PT2x1 = tau2x1*U2x1*( H2x1*U2x1*( U[i] - U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i]-B[i-1] )/hx - H2x1*Force2x1 ) + g*tau2x1*( U2x1*( 0.5*H[i]*H[i]-0.5*H[i-1]*H[i-1] )/hx + H2x1*H2x1*( U[i]-U[i-1] )/hx );
				
				}

				B2x2 = 0.5*(B[i+1] + B[i]);
				//
				H2x2 = 0.5*(H[i+1] + H[i]);
				//

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
				}
				
				//
				Force2x2 = 0.5*( Force[i] + Force[i+1] );
				//Force2x1 = 0.5*( Force[i-1] + Force[i] );


				//J2x2
				J2x2 = H2x2*U2x2 - tau2x2*( ( H[i+1]*U[i+1]*U[i+1] - H[i]*U[i]*U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1] - B[i] )/hx - H2x2*Force2x2 );

				
				//Ht
				Ht[i] = H[i] - (dT/hx)*( J2x2 - J2x1 ); 

				
				//PT2x2
				PT2x2 = tau2x2*U2x2*( H2x2*U2x2*( U[i+1] - U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1]-B[i] )/hx - H2x2*Force2x2 ) + g*tau2x2*( U2x2*( 0.5*H[i+1]*H[i+1]-0.5*H[i]*H[i] )/hx + H2x2*H2x2*( U[i+1]-U[i] )/hx );
				
				//PT2x1
				//PT2x1 = tau2x1*U2x1*( H2x1*U2x1*( U[i] - U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i]-B[i-1] )/hx - H2x1*Force2x1 ) + g*tau2x1*( U2x1*( 0.5*H[i]*H[i]-0.5*H[i-1]*H[i-1] )/hx + H2x1*H2x1*( U[i]-U[i-1] )/hx );
				

				if (Ht[i] < 0.0)
				{				
					stop = 1;
				}
				
				if (Ht[i] > epsilon)
				{					
					// H -> 0.5*(H2x2+H2x1)
					Ut[i] = ( H[i]*U[i] + (dT/hx)*( PT2x2-PT2x1 ) - (dT/hx)*( U2x2*J2x2-U2x1*J2x1 ) - g*(dT/hx)*( 0.5*H2x2*H2x2-0.5*H2x1*H2x1 ) + dT*( 0.5*(H2x2+H2x1) - tau[i]*( H2x2*U2x2-H2x1*U2x1 )/hx )*( Force[i] - g*( B2x2-B2x1 )/hx ) )/Ht[i];
									
				}
				else
				{
					Ut[i] = 0.0;
					
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
			        
			//Граничные точки
			
			// Задача с сухим дном на холмике
			Ht[0] = Ht[1];      Ht[Nx-1] = Ht[Nx-2];
			
			Ut[0] = -Ut[1];     Ut[Nx-1] = -Ut[Nx-2];
			

			
			for (i=0; i<Nx; i++)
			{
				H[i] = Ht[i];
				U[i] = Ut[i];

				if (Ht[i]>epsilon)
				{					
					tau[i] = alpha*hx/sqrt(g*H[i]);
				}
				else
				{
					tau[i] = 0.0;
				}

			}
			///////////////////////////////////////        
        
			// Вывот процентного соотношения выволненнных расчетов 
			// в консольную строку			
			
			Percentage_N = int(100*T_circle/T_add);
			if (Percentage_N >= dN)
			{	
				cout << Percentage_N << "%\n";
				dN += 1;
			}

			
			//**************************************************************************
			// Вывести промежуточные результаты во внешний файл	
			//**************************************************************************
			
			if (T_export_graphic_start <= T_circle && T_circle < T_export_graphic_end)			
			{
							

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
			

				Buffer_Name_Tecplot += ".dat";
												
				f_id_tecplot = fopen(Buffer_Name_Tecplot.c_str(),"w");

				// Очищаем сроку от символов с названием файла
				Buffer_Name_Tecplot.erase();

							
				// "VARIABLES = X,H,U \n"
				
				
				//TEMP
				// Delta = H(t) - H(0)
				N_smb = sprintf( buffer_name,"VARIABLES = X,Ksi,B,H,U,tau \n");

				fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

								
				// "ZONE T=(...), I=(...), DATAPACKING=POINT\n"
				
				N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", I=%d, DATAPACKING=POINT\n",T_export_graphic_start+T_current, Nx);
				fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
					
				
				for (i=0; i<Nx; i++)
				{
					Ksi = H[i] + B[i];
					N_smb = sprintf( buffer_name,"%f\t%f\t%f\t", X[i], Ksi, B[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
										
					N_smb = sprintf( buffer_name,"%f\t%f\t%f\n", H[i], U[i], tau[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

				}

				
				fclose(f_id_tecplot);
			
				// 
				T_export_graphic_start += step_export_graphic;

			}
			// конец условия if (T_circle >= T_export_graphic)
			
		}
		// конец if (stop==0)

		
			
	}
	//конец while (T_circle < T_add)
	
        
			
    return 0;
} 