

#include <math.h>
#include <time.h>
#include <stdio.h>

//#include <iostream.h>
//#include <iomanip>

#include <algorithm>
#include <iostream>
#include <fstream>
using namespace std;

//#define ANIMATION
#define TECPLOT_EXPORT
//#define BINARY_EXPORT
//#define VISCOSITY_TERM
//#define SMOOTH

/* Все должно работать следующим образом.
Если задан #define ANIMATION то в код будет включен экспорт анимации

	TECPLOT_EXPORT - в файл для TecPlot записываются отдельные временные слои
	BINARY_EXPORT - в бинарный файл записывается какой-то временной слой (не предусмотрена
запись нескольких файлов. Это и не нужно)
*/

//количество запоминаемых значащих цифр
#define NDEC 10

//#define MIN_X 0.01


//**************************************************************************
//{Eta} = 5000 - {H} - {B}
//{Fr} = abs({U})/sqrt(9.8*{H})
//**************************************************************************

int main(int argn, char **argv) 
{ 

	//**************************************************************************
	//	Инициализация переменных
	//**************************************************************************

	// Мне нужно вывести числа в 16-ричной кодировке
	long fl;
	
	float TEMP;
	
	double g = 9.8;
	int i,j;

	double T_add = 2.0 + 0.1; // T_add = 2.1
	double alpha = 0.5; // alpha = 0.3
	double beta = 0.01;	//
	
	
	
	double L = 100;
	int Nc = 2000;	
	double hx = L/Nc;
	int Nx = Nc+1;

	double MAX_H, MAX_U;

	double dT;

	double epsilon = 0.01; // min computational epsilon = 0.4

	double H0 = 1; // Аналог гамма

	
	double step_export_graphic = 0.5; // 
	double T_export_graphic_start = 0.0; // 
	double T_export_graphic_end = 1.5;
	

	double T_export_data = 2.0;

	// ********************************* Animation
	// Частота кадров анимации, которые будут записаны в файл, регулируется ТРЕМЯ параметрами
	// 1) T_animation_start - момент, с которого начинается анимация, может нужно с более позднего
	// 2) T_animation_end - момент, когда экспорт кадров/картинок заканчивается
	// 3) step_animation - шаг, с которым кадры экспортируются в файл
	// T_circle - текущее время, которое меняется, когда идет "расчетный" цикл, где проходят вычисления
	
	FILE* f_id_animation;

	double T_animation_start = 0.0;
	double T_animation_end = 4.0;
	double step_animation = 0.025;

	int Number_of_variables;


	

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

	double q, Q, Fr;

	int i_shore;	// переменная для хранения индекса побережья

	// T_current фиксирует время задачи. Оно берется из dat файла с данными
	double T_current = 0.0;

	// К величине T_circle прибавляется переменный шаг по времени dT
	double T_circle = 0.0;
	

	

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
	//	
	//**************************************************************************


	double* X = new double[Nx];
	
	double* H = new double[Nx];
	double* U = new double[Nx];
	double* B = new double[Nx];
	
	double* Force = new double[Nx];           
	double* tau = new double[Nx];

	double* Ht = new double[Nx];
	double* Ut = new double[Nx];


	//=========================================================================
	// Код для вывода потока J и других величин

	double* J_t = new double[Nx];
	
	//=========================================================================
	
	double h_L = 10.0;
	//double u_L = -5.0;
	//double u_L = -2.0*sqrt(g*h_L);
	double u_L = -25;

	double h_R = 10.0;
	//double u_R = 5.0;
	//double u_R = 2.0*sqrt(g*h_R);
	double u_R = 25;

	for (i=0; i<Nx; i++)
	{
		X[i] = i*hx;
		
		
		if (X[i]<L/2.0)
		{
			H[i] = h_L;
			U[i] = u_L;

		}
		else
		{
			H[i] = h_R;
			U[i] = u_R;

		}
		

		//U[i] = 0.0;

		B[i] = 0.0;
		Force[i] = 0.0;
		
	}	

	// ********************************* Начальные данные для тестов (из задач Беликова)
	// Первый u_l=0, b_l=3m, h_l=7m;//// u_r=0, b_r=0m, h_r=0.01m

	// Второй u_l=0, b_l=3m, h_l=7m;//// u_r=0, b_r=0m, h_r=1m

	// (*этот пока сделаю) Третий (плохой, т.к. для него обязательно добавка нужна) u_l=0, b_l=3m, h_l=7m;//// u_r=0, b_r=0m, h_r=4m

	// Четвертый u_l=0, b_l=0m, h_l=10m;///// u_r=0, b_r=3m, h_r=0.2m

	// Пятый u_l=0, b_l=0m, h_l=10m;///// u_r=0, b_r=3m, h_r=2m
	

	

	//**************************************************************************
	//	
	//**************************************************************************

	cout << "sizeof(double)  " << sizeof(double) << endl;

	
	for (i=0; i<Nx; i++)
	{
		if (H[i]>epsilon)
		{			
			//tau[i] = alpha*hx/sqrt(g*H[i]);
			// Modified formula for tau
			tau[i] = alpha*hx/(sqrt(g*(H[i]+H0)) + abs(U[i]));
		}
		else
		{			
			tau[i] = 0.0;
		}

		Force[i] = 0.0;

		
	}

	#ifdef ANIMATION
	//**************************************************************************
	//	Создание файла для АНИМАЦИИ, запись первой строки
	//**************************************************************************

	// При каждом старте программы предыдущее содержимое Animation_Ksi.dat удаляется
	f_id_animation = fopen("Animation_Ksi.dat","w");

	// Особенность формата DATAPACKING=BLOCK, нужно перечислять VARIABLES
	// Их очень много, но они заранее известны. 

	//ВНИМАНИЕ, ПРАВИЛА ДЛЯ ОБЛЕГЧЕНИЯ ЖИЗНИ
	// T_animation_end должна быть включенная в расчетные цикл, т.е. T_animation_end < окончательное время расчета

	
	
	Number_of_variables = int((T_animation_end-T_animation_start)/step_animation);
	
	cout << "Number_of_variables = " << Number_of_variables << endl;
	
	//
	Buffer_Name_Tecplot += "VARIABLES = X";

	for (i=1; i<=Number_of_variables; i++)
	{
		Buffer_Name_Tecplot += ",V";
			p_number = gcvt(i,NDEC,buffer_name);
		Buffer_Name_Tecplot += p_number;
	}

	Buffer_Name_Tecplot += "\n";

	N_smb = sprintf( buffer_name,Buffer_Name_Tecplot.c_str());
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_animation);

	Buffer_Name_Tecplot.erase();

	
	//*********************************************************************
	
	
	
	N_smb = sprintf( buffer_name,"ZONE T=\"Animation of Ksi\", I=%d, DATAPACKING=BLOCK\n", Nx-1);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_animation);

	for (i=0; i<(Nx-1); i++)
	{
		N_smb = sprintf( buffer_name,"%f\t", X[i]);
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_animation);
	}

	N_smb = sprintf( buffer_name,"\n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_animation);

	fclose(f_id_animation);

	#endif

	//**************************************************************************
	//	Основной цикл по времени while (T_circle < T_add)
	//**************************************************************************

	//TEMP
	int number_circle = 1;

	//TEMP
	//int number_for_data_export = 1;

	

	// TEMP
	double Temp_variable;


	/*
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
	*/


	// К величине T_circle прибавляется переменный шаг по времени dT
	while (T_circle < T_add)
	{
		
		MAX_H = 0.0;
		MAX_U = 0.0;
		for (i=0; i<Nx; i++)
		{
			if (MAX_H < H[i])	MAX_H = H[i];
			if (MAX_U < abs(U[i]))	MAX_U = abs(U[i]);
		}
		
		dT = (beta*hx)/(sqrt(g*MAX_H) + MAX_U);
		
		

		T_circle += dT;
		number_circle += 1;
		
	
		// Если в процессее расчета Ht<0, то stop = 1
		if (stop==0)
		{       
			for (i=1; i<(Nx-1); i++)
			{
				/*
				if (B[i]==0.0 && B[i-1]==3.0)
				{
					B2x1 = B[i-1];	
					B2x2 = B[i];	//or B[i+1]
				
				}
				else
				{
					if (B[i]==3.0 && B[i+1]==0.0)
					{	
						B2x1 = B[i-1];	//or B[i-1]
						B2x2 = B[i];
				
					}
				}
				*/
				B2x1 = 0.5*(B[i] + B[i-1]);
				B2x2 = 0.5*(B[i+1] + B[i]);
				
			
				
				if (i==1)
				{
					//B2x1 = 0.5*(B[i] + B[i-1]);	// вынесем за скобки if(i==1)
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
				
					J_t[0] = J2x1;
				

				}

				// Достаточно для каждого i вычислять 2x2

								
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
				
				J_t[i] = J2x2;
										
				//Ht
				Ht[i] = H[i] - (dT/hx)*( J2x2 - J2x1 ); 

				//PT2x2 = tau2x2*U2x2*( H2x2*U2x2*( U[i+1] - U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1]-B[i] )/hx - H2x2*Force2x2 ) + g*tau2x2*( U2x2*( 0.5*H[i+1]*H[i+1]-0.5*H[i]*H[i] )/hx + H2x2*H2x2*( U[i+1]-U[i] )/hx );
				
				// Additional member ~ tau*p*du/dx
				//PT2x2 = tau2x2*U2x2*( H2x2*U2x2*( U[i+1] - U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1]-B[i] )/hx - H2x2*Force2x2 ) + g*tau2x2*( U2x2*( 0.5*H[i+1]*H[i+1]-0.5*H[i]*H[i] )/hx + H2x2*H2x2*( U[i+1]-U[i] )/hx ) + tau2x2*g*0.5*H2x2*H2x2*( U[i+1]-U[i] )/hx;
				
				
								
				#if defined VISCOSITY_TERM
				// Additional member ~ tau*p*du/dx
				PT2x2 = tau2x2*U2x2*( H2x2*U2x2*( U[i+1] - U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1]-B[i] )/hx - H2x2*Force2x2 ) + g*tau2x2*( U2x2*( 0.5*H[i+1]*H[i+1]-0.5*H[i]*H[i] )/hx + H2x2*H2x2*( U[i+1]-U[i] )/hx ) + tau2x2*g*0.5*H2x2*H2x2*( U[i+1]-U[i] )/hx;
				
				#else
				//PT2x2
				PT2x2 = tau2x2*U2x2*( H2x2*U2x2*( U[i+1] - U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1]-B[i] )/hx - H2x2*Force2x2 ) + g*tau2x2*( U2x2*( 0.5*H[i+1]*H[i+1]-0.5*H[i]*H[i] )/hx + H2x2*H2x2*( U[i+1]-U[i] )/hx );
								
				#endif
				

				//====================================NEVER USE
				//PT2x1
				//PT2x1 = tau2x1*U2x1*( H2x1*U2x1*( U[i] - U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i]-B[i-1] )/hx - H2x1*Force2x1 ) + g*tau2x1*( U2x1*( 0.5*H[i]*H[i]-0.5*H[i-1]*H[i-1] )/hx + H2x1*H2x1*( U[i]-U[i-1] )/hx );
				//====================================NEVER USE

						
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

		
		
			//Граничные точки
			Ht[0] = Ht[1];
			Ut[0] = Ut[1];

			Ht[Nx-1] = Ht[Nx-2];
			Ut[Nx-1] = Ut[Nx-2];

			for (i=0; i<Nx; i++)
			{
				H[i] = Ht[i];
				//U[i] = Ut[i];

				if (H[i]>epsilon)
				{					
					//tau[i] = alpha*hx/sqrt(g*H[i]);
					// Modified formula for tau
					tau[i] = alpha*hx/(sqrt(g*(H[i]+H0)) + abs(U[i]));

					U[i] = Ut[i];
				}
				else
				{					
					tau[i] = 0.0;

					U[i] = 0.0;
					
				}

			}

			///////////////////////////////////////   
		

			
        
			//**************************************************************************
			//	Вывот процентного соотношения выволненнных расчетов в консольную строку		
			//**************************************************************************
			
			Percentage_N = int(100*T_circle/T_add);
			if (Percentage_N >= dN)
			{	
				cout << Percentage_N << "%\n";
				dN += 1;
			}

			#ifdef ANIMATION

			// ********************************* Animation START
			// Написал специальный код. Теперь можно в TecPlot строить одномерную анимацию
			
			if (T_animation_start <= T_circle && T_circle < T_animation_end)
			{
				
				f_id_animation = fopen("Animation_Ksi.dat","a");

				for (i=0; i<(Nx-1); i++)
				{
					TEMP = H[i] + B[i];
					
					N_smb = sprintf( buffer_name,"%f\t", TEMP);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_animation);		

					// ИНТЕРЕСНО, а можно найти такой редактор, программу, которая отслеживает сделанные изменения в программе
				}

				N_smb = sprintf( buffer_name,"\n");
				fwrite(&buffer_name, sizeof(char), N_smb, f_id_animation);

				fclose(f_id_animation);


				T_animation_start += step_animation;	

				// ДВЕ ВЕЩИ
				//1) нужен счетчик числа временных слоев, которые вышли в файл
				//2) Пишет, что нужно дописать variables, если формат не POINT ПРОБЛЕМКА. Заранее не известно сколько variables
			}



			// ********************************* Animation END
			#endif

			#ifdef TECPLOT_EXPORT

			//**************************************************************************
			//	Вывод данных в файл TecPlot	
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

				// Файл с неким сконструированным именем создается, ему присваивается индентификатор f_id_tecplot
												
				f_id_tecplot = fopen(Buffer_Name_Tecplot.c_str(),"w");
				
				
				// "VARIABLES = X,H,U \n"

				// Добавляем новые переменные q=hu, Q=0.5u^2+gb
				
				N_smb = sprintf( buffer_name,"VARIABLES = X,Ksi,B,H,U,tau\n");
				fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
				
				// "ZONE T=(...), I=(...), DATAPACKING=POINT\n"
				
				N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", I=%d, DATAPACKING=POINT\n",T_export_graphic_start+T_current, Nx-1);
				fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
							
				#if defined SMOOTH

				for (i=0; i<Nx; i++)
				{
					if (i!=0 && i!=(Nx-1))
					{						
						//заменим
						// H[i] -> 0.25*(H[i-1] + 2*H[i] + H[i+1])							
						// U[i] -> 0.25*(U[i-1] + 2*U[i] + U[i+1])

						TEMP = 0.25*(H[i-1] + 2*H[i] + H[i+1]) + B[i];


						N_smb = sprintf( buffer_name,"%f\t%f\t%f\t",X[i], TEMP, B[i]);
						fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
						
						N_smb = sprintf( buffer_name,"%f\t%f \t%f\n", 0.25*(H[i-1] + 2*H[i] + H[i+1]), 0.25*(U[i-1] + 2*U[i] + U[i+1]), tau[i]);
						fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
					}
					else
					{
						TEMP = H[i] + B[i];
						N_smb = sprintf( buffer_name,"%f\t%f\t%f\t",X[i], TEMP, B[i]);
						fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
						
						N_smb = sprintf( buffer_name,"%f\t%f \t%f\n", H[i], U[i], tau[i]);
						fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
					}
				}

				#else

				for (i=0; i<Nx; i++)
				{
					TEMP = H[i] + B[i];
					
					N_smb = sprintf( buffer_name,"%f\t%f\t%f\t",X[i], TEMP, B[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);		
			
					N_smb = sprintf( buffer_name,"%f\t%f \t%f\n", H[i], U[i], tau[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
				}

				#endif


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

			#endif

			#ifdef BINARY_EXPORT
			//**************************************************************************
			//	Вывод данных в бинарный файл
			//**************************************************************************
			
			/*
			if (T_circle<=T_export_data && T_export_data<(T_circle+dT))
			{
				// Формат бинарного файла
				// T_export_data alpha beta hx Nx
				// (i=0..Nx-1) H (i=0..Nx-1) U (i=0..Nx-1) B

				// Начинаем конструировать имя файла
				// "DataFlow Tf=(...).dat"
				
				// Tf
				Buffer_Name_Tecplot += "Binary_Numerical Tf=";
					p_number = gcvt(T_export_data+T_current,NDEC,buffer_name);
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

				fwrite(&Nx, sizeof(int), 1, f_id_tecplot);				//Nx
				TEMP = 34.4;
				fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);	//T_export_data
				TEMP = 34.5;
				fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);			//alpha
				TEMP = beta;
				fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);				//beta
				TEMP = hx;
				fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);				//hx
				
				/*
				fwrite(&Nx, sizeof(int), 1, f_id_tecplot);				//Nx
				fwrite(&T_export_data, sizeof(double), 1, f_id_tecplot);	//T_export_data
				fwrite(&alpha, sizeof(double), 1, f_id_tecplot);			//alpha
				fwrite(&beta, sizeof(double), 1, f_id_tecplot);				//beta
				fwrite(&hx, sizeof(double), 1, f_id_tecplot);				//hx
				*/

				/*
				for (i=0; i<Nx; i++)
				{
					TEMP = X[i];
					fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);				//X
					TEMP = H[i];					
					fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);				//H
					TEMP = U[i];					
					fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);				//U
					TEMP = B[i];					
					fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);				//B

					
				}
				*//*

				for (i=0; i<Nx; i++)
				{
					TEMP = X[i];
					fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);				//X

					if (i==345)
					{
						fl = cout.flags();
						cout<<"Initial flags:"<<fl<<endl;

						cout.setf(ios::hex);

						fl = cout.flags();
						cout<<"Change flags:"<<fl<<endl;

						
						cout << "i->" << hex << i << endl;
						//cout.unsetf(ios::hex);
						
						cout << "X[i]->" << hex << X[i] << endl;
						cout << "H[i]->" << H[i] << endl;
						cout << "U[i]->" << U[i] << endl;
						cout << "B[i]->" << B[i] << endl;
					
						TEMP = X[i];
						printf("\nExperiment %A\n\n",TEMP);
				
					}
				}

				for (i=0; i<Nx; i++)
				{
					TEMP = H[i];
					fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);				//H
				}

				for (i=0; i<Nx; i++)
				{
					TEMP = U[i];
					fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);				//U
				}

				for (i=0; i<Nx; i++)
				{
					TEMP = B[i];
					fwrite(&TEMP, sizeof(float), 1, f_id_tecplot);				//B
				}

				

				cout << "sizeof(double)->" << sizeof(double) << endl;
				cout << "sizeof(int)->" << sizeof(int) << endl;
				cout << "sizeof(float)->" << sizeof(float) << endl;

				fclose(f_id_tecplot);

				// Очищаем сроку от символов с названием файла
				Buffer_Name_Tecplot.erase();
			}
			*/

			if (T_circle<=T_export_data && T_export_data<(T_circle+dT))
			{
				// Записываем цифры, которые были получены в результате расчетов, 
				// в текстовом виде

				// Начинаем конструировать имя файла
				// "DataFlow Tf=(...).dat"
				
				// Tf
				Buffer_Name_Tecplot += "Result_text_Numerical Tf=";
					p_number = gcvt(T_export_data+T_current,NDEC,buffer_name);
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

				
				
				
				N_smb = sprintf( buffer_name,"%d\n%.10f\n%.10f\n%.10f\n%.10f\n", Nx, T_export_data, alpha, beta, hx);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

				
				for (i=0; i<Nx; i++)
				{					
					N_smb = sprintf( buffer_name,"%.10f\t%.10f\t%.10f\t%.10f\n", X[i], H[i], U[i], B[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);					
				}
				

				fclose(f_id_tecplot);

				// Очищаем сроку от символов с названием файла
				Buffer_Name_Tecplot.erase();
			}

			#endif
			
		}
		// конец if (stop==0)

			
			
	}
	//конец while (T_circle < T_add)

	/*
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
	*/

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
     
	
			
    return 0;
} 
