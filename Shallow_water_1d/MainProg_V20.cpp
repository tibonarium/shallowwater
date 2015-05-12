
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

/*
#!MC 1120
# Created by Tecplot 360 build 11.2-0-563
$!EXPORTSETUP EXPORTFORMAT = AVI				// просто говорит о том, что создается AVI файл
$!EXPORTSETUP EXPORTFNAME = "Nu_hotwall.avi"
$!EXPORTSTART
$!VARSET |count| = 1	// |count| - переменная $!VARSET - указывает, что |count| переменная
$!LOOP 30		// (?) Говорит, что следующие команды до ENDLOOP нужно выполнить 30 раз
$!VARSET |count| += 1
$!$!VARSET |FNAME| ="|count|.plt"
$!VARSET |TIME| = |count|
$!VARSET |TIME| *= 0.025

$!READDATASET "|FNAME|"

$!XYLINEAXIS XDETAIL 1 {TITLE{TITLEMODE = USETEXT}}
$!XYLINEAXIS XDETAIL 1 {TITLE{TEXT = 'Position (0.0, 1.0, 0.0) m'}}
$!XYLINEAXIS YDETAIL 1 {TITLE{TITLEMODE = USETEXT}}
$!XYLINEAXIS YDETAIL 1 {TITLE{TEXT = 'Nusselt Number (Hot Wall)'}}
$!XYLINEAXIS XDETAIL 1 {RANGEMIN = 0}
$!XYLINEAXIS XDETAIL 1 {RANGEMAX = 1}
$!XYLINEAXIS YDETAIL 1 {RANGEMAX = 25}
$!XYLINEAXIS YDETAIL 1 {RANGEMIN = -25}
$!ATTACHTEXT
ANCHORPOS
{
X = 53.9906103286385
Y = 85.03521126760563
}
TEXT = '|TIME| sec'
$!REDRAW
$!EXPORTNEXTFRAME
$!ENDLOOP
$!EXPORTFINISH
*/

//**************************************************************************
//	
//**************************************************************************

int main(int argn, char **argv) 
{ 
	/*
	B(x) =
			1.4 - 0.001*x ,	x < 100
			1.3 - 0.01*(x-100) ,	100 <= x <= 200
			0.3 - 0.001*(x-200) ,	x > 200
	
	L = 500 m
	H0 = 1.75 m
	H(L) = 1 + 0.75*cos(2*pi*t/T);
	T = 60*60 s = 1200 s

	12 min = 12*60 = 720 s		/4 = 180
	24 min = 24*60 = 1440 s		/4 = 360
	36 min = 36*60 = 2160 s		/4 = 540
	48 min = 48*60 = 2880 s		/4 = 720
	54 min = 54*60 = 3240 s		/4 = 810

	(for interest)
	alpha = 0.1
	beta = 0.2
	hm! N = 100

	Fric = - g*nu*nu*U*abs(U)/pow(H,4/3);
	nu = 0.03;

	1) R = pow(X, Y)
	2) R = exp(Y * log(X))

	page 25
	
	  delta = 0.1;
	  epsilon = 0.001
	
	  gamma = delta*MAX_H0;

  teta_i =	alpha*dx/ci				hi>=epsilon
			alpha*dx/(ci+gamma)		hi<epsilon

  dT =	beta*dx/cmax				hmax>=epsilon
		beta*dx/(cmax+gamma)		hmax<epsilon

	Video 
	35 sec * 24 cards = 840
	
	*/

	//**************************************************************************
	//	Другие сложности
	//**************************************************************************

	//$! означает макрокоманду для Текплота

	// {Ksi} = {H}+{B}

	//**************************************************************************
	//	Инициализация переменных
	//**************************************************************************

	const double pi = 3.1415;

	double g = 9.8;
	int i,j,k;

	double T0 = 3300; // 3300 s
	double alpha = 0.1; // 0.1 
	double beta = 0.2; // 0.2
		
	
	double L = 500; // 500m
	int Nx = 501;
	double hx = 1; //m


	double epsilon = 0.001;
	//double gamma;

	double nu;
	

	// dT = beta*hx/sqrt(9.8*H_MAX) = (0.2*5)/sqrt(9.8*1.75) = 1/sqrt(17.15) = 0.24

	double step_export_graphic = 120; //2*60 // 4s 35 sec*24 = 840 cards -> *4 = 3360 sec 
	
	double T_export_graphic_start = 720;
	double T_export_graphic_end = 3240;

	double T_export_data = 3240; // 3240 s

	double* X = new double[Nx];
	
	double* H = new double[Nx];
	double* U = new double[Nx];
	double* B = new double[Nx];
	
	double* Force = new double[Nx];           
	double* tau = new double[Nx];

	//TEMP
	double* Jm = new double[Nx-1];

	double* Ht = new double[Nx];
	double* Ut = new double[Nx];

	//TEMP
	double* Ht_c = new double[Nx];
	double* Ut_c = new double[Nx];

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
	// Указатель, который используем для обозначения файла с несколькими временными слоями
	FILE* f_id_1D_FLOW;

	
	//TEMP
	FILE* f_id_tecplot_TEMP;

	
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
		//B[j] = 0.0;
		//B[j] = max(0,0.25-5*(X[j]-0.5)*(X[j]-0.5));

		/*
		if (0.25-5*(X[j]-0.5)*(X[j]-0.5) > 0.0)
		{
			B[j] = 0.25-5*(X[j]-0.5)*(X[j]-0.5);

		}
		else
		{
			B[j] = 0.0;
		}
		*/


		
		if (X[j] < 100)
		{
			B[j] = 1.4 - 0.001*X[j];
		}
		else
		{
			if(X[j] <= 200)
			{
				B[j] = 1.3 - 0.01*(X[j]-100);
			}
			else
			{
				B[j] = 0.3 - 0.001*(X[j]-200);
			}
		}
		
		
	}
	


	for (i=0; i<Nx; i++)
	{
		
		H[i] = 1.75 - B[i];//m
		

		/*
		if (B[i] >= 0.1)
		{
			H[i] = epsilon;

		}
		else
		{
			///H[i] = 0.1 + 0.005*sin((2*pi)*X[i]/0.1) - B[i];
			H[i] = 0.1 - B[i];
		}
		*/

		U[i] = 0.0;
		Force[i] = 0.0;
		//TEMP
		Jm[i] = 0.0;
	}	
	


	
	
	
	//**************************************************************************
	//	Открываем и записываем первую зону в файл 1D_Data_Flow.dat
	//**************************************************************************

	f_id_1D_FLOW = fopen("1D_Data_Flow.dat","w");

	N_smb = sprintf( buffer_name,"TITLE=\"1D Test Seashore Periodical Wave\" \n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_1D_FLOW);
		
	N_smb = sprintf( buffer_name,"VARIABLES = X,H,U,B \n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_1D_FLOW);
	
	if (T_current == 0.0)
	{		

		// "ZONE T=(...), I=(...), DATAPACKING=POINT\n"
				
		N_smb = sprintf( buffer_name,"ZONE T=\"Time 0.0\", I=%d, DATAPACKING=POINT, SOLUTIONTIME=%f, STRANDID=1\n", Nx, 0.0);
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_1D_FLOW);

		for (j=0; j<Nx; j++)
		{
			N_smb = sprintf( buffer_name,"%f\t%f\t%f\t%f\n", X[j], H[j], U[j], B[j]);
			fwrite(&buffer_name, sizeof(char), N_smb, f_id_1D_FLOW);			
		}

		N_smb = sprintf( buffer_name,"\n");
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_1D_FLOW);
	}

	fclose(f_id_1D_FLOW);
	

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


		nu = 0.03;
		

		// TEMP
		for (i=0; i<Nx; i++)
		{
			if (H[i]>epsilon)
			{
				Force[i] = -g*nu*nu*U[i]*fabs(U[i])/pow(H[i],4/3); // Fric = - g*nu*nu*U*abs(U)/pow(H,4/3); 	nu = 0.03;
				//Force[i] = -g*nu*nu*U[i]*fabs(U[i])/pow(pow(H[i],1./3),4);
			}
			else
			{
				Force[i] = -g*nu*nu*U[i]*fabs(U[i])/pow(epsilon,4/3);
				//Force[i] = -g*nu*nu*U[i]*fabs(U[i])/pow(pow(epsilon,1./3),4);
			}
		}
		
		
		
		
	
		// Если в процессее расчета Ht<0, то stop = 1
		if (stop==0)
		{       
			for (i=1; i<(Nx-1); i++)
			{

				if (i==1)
				{
					B2x1 = 0.5*(B[i] + B[i-1]);
					H2x1 = 0.5*(H[i] + H[i-1]);

					// TEMP
					if (H2x1>epsilon)
					{				
						U2x1 = 0.5*(U[i] + U[i-1]);
						tau2x1 = 0.5*(tau[i] + tau[i-1]);

						//TEMP
						Force2x1 = -g*nu*nu*U2x1*fabs(U2x1)/pow(H2x1,4/3);
					}
					else
					{
						U2x1 = 0.0;
						tau2x1 = 0.0;

						//TEMP
						Force2x1 = -g*nu*nu*U2x1*fabs(U2x1)/pow(epsilon,4/3);
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

					//TEMP
					Force2x2 = -g*nu*nu*U2x2*fabs(U2x2)/pow(H2x2,4/3);
				}
				else
				{					
					U2x2 = 0.0;
					tau2x2 = 0.0;

					//TEMP
					Force2x2 = -g*nu*nu*U2x2*fabs(U2x2)/pow(epsilon,4/3);
				}
				

				//U2x2 = 0.5*(U[i+1] + U[i]);
				//U2x1 = 0.5*(U[i] + U[i-1]);

				//tau2x2 = 0.5*(tau[i+1] + tau[i]);
				//tau2x1 = 0.5*(tau[i] + tau[i-1]);


				//
				//Force2x2 = 0.5*( Force[i] + Force[i+1] );
				//Force2x1 = 0.5*( Force[i-1] + Force[i] );


				//J2x2
				J2x2 = H2x2*U2x2 - tau2x2*( ( H[i+1]*U[i+1]*U[i+1] - H[i]*U[i]*U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1] - B[i] )/hx - H2x2*Force2x2 );
				
				//J2x1
				//J2x1 = H2x1*U2x1 - tau2x1*( ( H[i]*U[i]*U[i] - H[i-1]*U[i-1]*U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i] - B[i-1] )/hx - H2x1*Force2x1 );
				

				//TEMP
				if (i==(Nx-2))
				{
					Jm[i-1] = J2x1;
					Jm[i] = J2x2;
				}
				else
				{
					Jm[i-1] = J2x1;
				}
				
				
				//Ht
				Ht[i] = H[i] - (dT/hx)*( J2x2 - J2x1 ); 

				
				
				//PT2x2
				PT2x2 = tau2x2*U2x2*( H2x2*U2x2*( U[i+1] - U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1]-B[i] )/hx - H2x2*Force2x2 ) + g*tau2x2*( U2x2*( 0.5*H[i+1]*H[i+1]-0.5*H[i]*H[i] )/hx + H2x2*H2x2*( U[i+1]-U[i] )/hx );
				
				//PT2x1
				//PT2x1 = tau2x1*U2x1*( H2x1*U2x1*( U[i] - U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i]-B[i-1] )/hx - H2x1*Force2x1 ) + g*tau2x1*( U2x1*( 0.5*H[i]*H[i]-0.5*H[i-1]*H[i-1] )/hx + H2x1*H2x1*( U[i]-U[i-1] )/hx );
				
		

				// !!!!!!!!!!!!!!!!TEMP
				if (Ht[i] < 0.0)
				//if (number_circle==1)
				{
					
					stop = 1;

					cout << "Very bad" << "\ni= " << i << "\nX_i= " << X[i] << "\nHt[i]= " << Ht[i] << "\nH[i]= " << H[i] << endl;
					// i=24 23*5 = 115 m
					// dH = H[i] - Ht[i] < H[t]
					cout << "number_circle= " << number_circle << "\nT_circle= " << T_circle << endl;
					cout << "\nLook at U" << endl;
					cout << "U[i-1]= " << U[i-1] << "\nU[i]= " << U[i] <<"\nU[i+1]= " << U[i+1] << endl;
					cout << "\nLook at H" << endl;
					cout << "H[i-1]= " << H[i-1] << "\nH[i]= " << H[i] <<"\nH[i+1]= " << H[i+1] << endl;
					
					cout << "\nMore Info\n" << endl;
					cout << ( J2x2 - J2x1 ) << "\n" << J2x2 << "\n" << J2x1 << "\n" << endl;

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

					Temp_variable = (dT/hx)*( PT2x2-PT2x1 );	
					cout << "\n\nUt\n\n" << Temp_variable << endl;

					Temp_variable = -(dT/hx)*( U2x2*J2x2-U2x1*J2x1 );	
					cout << Temp_variable << endl;

					Temp_variable = -g*(dT/hx)*( 0.5*H2x2*H2x2-0.5*H2x1*H2x1 );	
					cout << Temp_variable << endl;

					Temp_variable = dT*( 0.5*(H2x2+H2x1) - tau[i]*( H2x2*U2x2-H2x1*U2x1 )/hx )*( Force[i] - g*( B2x2-B2x1 )/hx );	
					cout << Temp_variable << endl;

					Temp_variable = tau[i];	
					cout << Temp_variable << endl;

					Temp_variable = g*dT/hx;	
					cout << Temp_variable << endl;

					Temp_variable = -dT*0.5*(H2x2+H2x1)*g*( B2x2-B2x1 )/hx;	
					cout << Temp_variable << endl;

					Temp_variable = -( H2x2*H2x2-H2x1*H2x1 );	
					cout << Temp_variable << endl;

					Temp_variable = -(H2x2+H2x1)*( B2x2-B2x1 );	
					cout << Temp_variable << endl;

					
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

			//TEMP
			///////////////////////////////////////////////////////////////////

			nu = 0.0;

			for (i=1; i<(Nx-1); i++)
			{

				if (i==1)
				{
					B2x1 = 0.5*(B[i] + B[i-1]);
					H2x1 = 0.5*(H[i] + H[i-1]);

					// TEMP
					if (H2x1>epsilon)
					{				
						U2x1 = 0.5*(U[i] + U[i-1]);
						tau2x1 = 0.5*(tau[i] + tau[i-1]);

						//TEMP
						Force2x1 = -g*nu*nu*U2x1*fabs(U2x1)/pow(H2x1,4/3);
					}
					else
					{
						U2x1 = 0.0;
						tau2x1 = 0.0;

						//TEMP
						Force2x1 = -g*nu*nu*U2x1*fabs(U2x1)/pow(epsilon,4/3);
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

					//TEMP
					Force2x2 = -g*nu*nu*U2x2*fabs(U2x2)/pow(H2x2,4/3);
				}
				else
				{					
					U2x2 = 0.0;
					tau2x2 = 0.0;

					//TEMP
					Force2x2 = -g*nu*nu*U2x2*fabs(U2x2)/pow(epsilon,4/3);
				}
				

				//J2x2
				J2x2 = H2x2*U2x2 - tau2x2*( ( H[i+1]*U[i+1]*U[i+1] - H[i]*U[i]*U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1] - B[i] )/hx - H2x2*Force2x2 );
				
				//J2x1
				//J2x1 = H2x1*U2x1 - tau2x1*( ( H[i]*U[i]*U[i] - H[i-1]*U[i-1]*U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i] - B[i-1] )/hx - H2x1*Force2x1 );
				

				//TEMP
				if (i==(Nx-2))
				{
					Jm[i-1] = J2x1;
					Jm[i] = J2x2;
				}
				else
				{
					Jm[i-1] = J2x1;
				}
				
				
				//Ht
				Ht_c[i] = H[i] - (dT/hx)*( J2x2 - J2x1 ); 

				
				
				//PT2x2
				PT2x2 = tau2x2*U2x2*( H2x2*U2x2*( U[i+1] - U[i] )/hx + g*( 0.5*H[i+1]*H[i+1] - 0.5*H[i]*H[i] )/hx + g*H2x2*( B[i+1]-B[i] )/hx - H2x2*Force2x2 ) + g*tau2x2*( U2x2*( 0.5*H[i+1]*H[i+1]-0.5*H[i]*H[i] )/hx + H2x2*H2x2*( U[i+1]-U[i] )/hx );
				
				//PT2x1
				//PT2x1 = tau2x1*U2x1*( H2x1*U2x1*( U[i] - U[i-1] )/hx + g*( 0.5*H[i]*H[i] - 0.5*H[i-1]*H[i-1] )/hx + g*H2x1*( B[i]-B[i-1] )/hx - H2x1*Force2x1 ) + g*tau2x1*( U2x1*( 0.5*H[i]*H[i]-0.5*H[i-1]*H[i-1] )/hx + H2x1*H2x1*( U[i]-U[i-1] )/hx );
				
		

				// !!!!!!!!!!!!!!!!TEMP
				if (Ht_c[i] < 0.0)
				//if (number_circle==1)
				{
					
					stop = 1;
					
				}
				
				if (Ht_c[i] > epsilon)
				{					
					// H -> 0.5*(H2x2+H2x1)
					Ut_c[i] = ( H[i]*U[i] + (dT/hx)*( PT2x2-PT2x1 ) - (dT/hx)*( U2x2*J2x2-U2x1*J2x1 ) - g*(dT/hx)*( 0.5*H2x2*H2x2-0.5*H2x1*H2x1 ) + dT*( 0.5*(H2x2+H2x1) - tau[i]*( H2x2*U2x2-H2x1*U2x1 )/hx )*( Force[i] - g*( B2x2-B2x1 )/hx ) )/Ht[i];
										
				}
				else
				{
					Ut_c[i] = 0.0;
					
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

			//Сравнить направление скорости

			for (i=1; i<(Nx-1); i++)
			{
				if (Ut[i]<0)
				{
					if (Ut_c[i]>0)
					{
						cout << "v1 Ep with F_fric Ut change direction" << endl;
					}
				}

				if (Ut[i]>0)
				{
					if (Ut_c[i]<0)
					{
						cout << "v2 Ep with F_fric Ut change direction" << endl;
					}
				}

			}


			///////////////////////////////////////////////////////////////////
			        
			//Граничные точки
			
			// Набегание волны на берег
			Ht[0] = Ht[1];      Ht[Nx-1] = 2*(1 + 0.75*cos(2*pi*T_circle/3600)) - Ht[Nx-2];
			//Ht[Nx-1] =  Ht[Nx-2];
			Ut[0] = -Ut[1];     Ut[Nx-1] = Ut[Nx-2];
			

			
			for (i=0; i<Nx; i++)
			{
				H[i] = Ht[i];
				U[i] = Ut[i];

				if (Ht[i]>epsilon)
				{
					//tau[i] = alpha*hx/sqrt(g*(H[i]+gamma));
					tau[i] = alpha*hx/sqrt(g*H[i]);
				}
				else
				{
					//tau[i] = alpha*hx/sqrt(g*(H[i]+gamma));
					//tau[i] = alpha*hx/(sqrt(g*H[i])+MAG*gamma);
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


			if (T_export_graphic_start <= T_circle && T_circle < T_export_graphic_end)			
			//if (number_circle==1)
			{
							

				// Начинаем конструировать имя файла
				// "Tecplot T0=(T_export_graphic) alpha=(...) beta=(...) hx=(...).dat"
				
				
				// T0
				Buffer_Name_Tecplot += "Tecplot T0=";
					p_number = gcvt((T_export_graphic_start+T_current)/60,NDEC,buffer_name);
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

				// Очищаем сроку от символов с названием файла
				Buffer_Name_Tecplot.erase();

				// TEMP "Jm T0=(...).dat"
				Buffer_Name_Tecplot += "Jm T0=";
					p_number = gcvt((T_export_graphic_start+T_current)/60,NDEC,buffer_name);
				Buffer_Name_Tecplot += p_number;
								
				
				Buffer_Name_Tecplot += ".dat";
				
				f_id_tecplot_TEMP = fopen(Buffer_Name_Tecplot.c_str(),"w");

				Buffer_Name_Tecplot.erase();
				
				
				// "VARIABLES = X,H,U \n"
				
				//N_smb = sprintf( buffer_name,"VARIABLES = X,B,H,U \n");
				//N_smb = sprintf( buffer_name,"VARIABLES = X,H \n");

				//TEMP
				N_smb = sprintf( buffer_name,"VARIABLES = X,Ksi,B,H,U,tau \n");

				fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

				//TEMP
				N_smb = sprintf( buffer_name,"VARIABLES = X,Jm \n");
				fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot_TEMP);
				
				// "ZONE T=(...), I=(...), DATAPACKING=POINT\n"
				
				N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", I=%d, DATAPACKING=POINT\n",(T_export_graphic_start+T_current)/60.0, Nx);
				fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

				//TEMP
				N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", I=%d, DATAPACKING=POINT\n",(T_export_graphic_start+T_current)/60.0, Nx);
				fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot_TEMP);

			
				
				for (i=0; i<Nx; i++)
				{
					Ksi = H[i] + B[i];
					N_smb = sprintf( buffer_name,"%f\t%f\t%f\t", X[i], Ksi, B[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
					
					
					N_smb = sprintf( buffer_name,"%f\t%f\t%f\n", H[i], U[i], tau[i]);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

					//TEMP
					N_smb = sprintf( buffer_name,"%f\t%f\n", (X[i]+0.5*hx), Jm[i]*100000);
					fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot_TEMP);

				}

				
				fclose(f_id_tecplot);
			
				//TEMP
				fclose(f_id_tecplot_TEMP);
							

				// 
				T_export_graphic_start += step_export_graphic;

			}
			// конец условия if (T_circle >= T_export_graphic)
			
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

	f_id_tecplot = fopen("Final_Flow.dat","w");	
	
	// "VARIABLES = X,H,U \n"
				
	N_smb = sprintf( buffer_name,"VARIABLES = X,Ksi,B,H,U \n");
	
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
				
	// "ZONE T=(...), I=(...), DATAPACKING=POINT\n"
		
	N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", I=%d, DATAPACKING=POINT\n",(T_export_graphic_start+T_current)/60.0, Nx);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

				
	for (i=0; i<Nx; i++)
	{
		Ksi = H[i] + B[i];
		
		N_smb = sprintf( buffer_name,"%f\t%f\t%f\t", X[i], Ksi, B[i] );
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
				
		N_smb = sprintf( buffer_name,"%f\t%f\n", H[i], U[i]);
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	}

	fclose(f_id_tecplot);
        
			
    return 0;
} 