// Physical_Geometry_Data.cpp
// Дата последней модификации: 29 января 2014

#include <QMessageBox>
#include <QFile>
#include <QTextStream>

#include <math.h>
#include <stdio.h>

#include "Physical_Geometry_Data.h"
#include "Geometry_Mesh.h"

#include <iostream>
using namespace std;

#define EMPTY_AREA			0
#define COMPUTATION_AREA	1
#define DOWN_WALL			2
#define UPPER_WALL			3
#define LEFT_WALL			4
#define RIGHT_WALL			5
#define DOWN_FREE			6
#define UPPER_FREE			7
#define LEFT_FREE			8
#define RIGHT_FREE			9
#define CORNER_POINT		10

// ВАЖНАЯ СТРОЧКА КОДА. Он говорит о том, что расчет будет идти на прямоугольных ячейках.
#define SQUARE_MESH

//#define TEMP_CODE



//**************************************************************************
// Конструктор TrianglePhysicalGeometryData(int nNode, int nElement)
//**************************************************************************

TrianglePhysicalGeometryData::TrianglePhysicalGeometryData() {
	Total_number_of_points = 0;
	Total_number_of_cells = 0;	
	Total_number_of_edges = 0;
}

//**************************************************************************
// Деструктор TrianglePhysicalGeometryData
//**************************************************************************

TrianglePhysicalGeometryData::~TrianglePhysicalGeometryData () {

	
	//////////////////////////
	Domain.~vector<Point>();
	DomainCell.~vector<TriangleCell>();
	DomainEdge.~vector<Edge>();

	
	H.~vector<double>();
	xU.~vector<double>();
	yU.~vector<double>();
	B.~vector<double>();
	
	tau.~vector<double>();
	epsilon.~vector<double>();
	
	ForceX.~vector<double>();
	ForceY.~vector<double>();
	
	Set_for_H_flow.~vector<double>();
	Set_for_H_ux_flow.~vector<double>();
	Set_for_H_uy_flow.~vector<double>();
	
	Set_for_B_dx.~vector<double>();
	Set_for_B_dy.~vector<double>();
	Set_for_H_ux_uy.~vector<double>();
}



void TrianglePhysicalGeometryData::size() {
	cout << "Total_number_of_points\t" << Total_number_of_points << endl;
	cout << "Total_number_of_cells\t" << Total_number_of_cells << endl;
	cout << "Total_number_of_edges\t" << Total_number_of_edges << endl;
	
	cout << "Domain.size()\t" << Domain.size() << endl;
	cout << "DomainCell.size()\t" << DomainCell.size() << endl;
	cout << "DomainEdge.size()\t" << DomainEdge.size() << endl;
}


double TrianglePhysicalGeometryData::getAverageLength() {
	return L_average;
}

//**************************************************************************
// ExportDataToTecplot() -> Запись данных в файл с указанным именем
//**************************************************************************

void TrianglePhysicalGeometryData::ExportDataToTecplot(const char* File_Name, double Time) 
{

	FILE* f_id_tecplot;
	char buffer_name[500];
	int N_smb;
	
	int k, m;	// Эти переменные нужны для циклов
			
	f_id_tecplot = fopen(File_Name, "w");

	//VARIABLES = X,Y,B,H,xU,yU
	N_smb = sprintf( buffer_name,"VARIABLES = X,Y,B,H,xU,yU,EPS,tau\n");	// Не забуть изменить "%f\t...\n"
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

	// ZONE T="Time 0.0", DATAPACKING=POINT, NODES=%d, ELEMENTS=%d, ZONETYPE=FETRIANGLE\n
	N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", DATAPACKING=POINT, NODES=%d, ELEMENTS=%d, ZONETYPE=FETRIANGLE\n", Time, Total_number_of_points, Total_number_of_cells);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	
	for (k=0; k<Total_number_of_points; k++)
	{
		if ( H[k]>epsilon[k] )
		{
			N_smb = sprintf( buffer_name,"%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", Domain[k].getX(), Domain[k].getY(), B[k], H[k], xU[k], yU[k], epsilon[k], tau[k]);
			fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
		}
		else
		{
			N_smb = sprintf( buffer_name,"%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", Domain[k].getX(), Domain[k].getY(), B[k], 0.0, xU[k], yU[k], epsilon[k], tau[k]);
			fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
		}
	}
								
	N_smb = sprintf( buffer_name,"\n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
				
	for (m=0; m<Total_number_of_cells; m++)
	{					
		N_smb = sprintf( buffer_name,"%d %d %d\n", DomainCell[m].getNode(FIRST_NODE)+1, DomainCell[m].getNode(SECOND_NODE)+1, DomainCell[m].getNode(THIRD_NODE)+1);
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	}
	
	N_smb = sprintf( buffer_name,"\n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	
	fclose(f_id_tecplot);

}

//**************************************************************************
// AddDataToTecplot() -> Добавляет данные в уже существующий файл Текплота
//**************************************************************************

void TrianglePhysicalGeometryData::AddDataToTecplot(const char* File_Name, double Time) {

	FILE* f_id_tecplot;
	char buffer_name[500];
	int N_smb;
	
	int k, m;	// Эти переменные нужны для циклов
	
	f_id_tecplot = fopen(File_Name, "a");
			
	// ZONE T="Time %f", DATAPACKING=POINT, NODES=%d, ELEMENTS=%d, ZONETYPE=FETRIANGLE, VARSHARELIST = ([1, 2]=1), CONNECTIVITYSHAREZONE = 1\n
	N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", DATAPACKING=POINT, NODES=%d, ELEMENTS=%d, ZONETYPE=FETRIANGLE, VARSHARELIST = ([1, 2, 3]=1), CONNECTIVITYSHAREZONE = 1\n", Time, Total_number_of_points, Total_number_of_cells);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	
	for (k=0; k<Total_number_of_points; k++)
	{
		if ( H[k]>epsilon[k] )
		{
			N_smb = sprintf( buffer_name,"%f\t%f\t%f\n", H[k], xU[k], yU[k]);
			fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
		}
		else
		{
			N_smb = sprintf( buffer_name,"%f\t%f\t%f\n", 0.0, xU[k], yU[k]);
			fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
		}
	}
								
	N_smb = sprintf( buffer_name,"\n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

	fclose(f_id_tecplot);

}

//**************************************************************************
// setDomain() -> Заполняем массив vector<Point> Domain
//**************************************************************************

// !?!?! Этот метод еще пригодится, когда будем превращать прямоугольную сетку в треугольную
void TrianglePhysicalGeometryData::setDomain(int Nx, int Ny, const double* X, const double* Y, const int* S) 
{

	// Внутренние переменные функции setDomain():
	int number_counter = 0;
	int i, j;

	// 1 - сначала подсчитаем общее число "активных" точек
	// под "активными" понимаю точки, которые являются внутренними или граничными
	for (i=0; i<Nx; i++)
	{
		for (j=0; j<Ny; j++)
		{
			if (S[i*Ny+j] != EMPTY_AREA)
			{
				number_counter += 1;
			}
		}
	}

	// 2 - Зная число "активных" точек, можно уже создать массив

	Total_number_of_points = number_counter;	// Присваиваем значение внутренней перменной
	
	Domain.reserve(Total_number_of_points);
		

	for (i=0; i<Nx; i++)
	{
		for (j=0; j<Ny; j++)
		{
			if (S[i*Ny+j] != EMPTY_AREA)
			{				
				TempPoint.setValues(X[i*Ny+j], Y[i*Ny+j], S[i*Ny+j]);
				Domain.push_back(TempPoint);

			}
		}
	}

}

//**************************************************************************
// ConstructEdges() -> Заполняем массив vector<SquareCell> DomainCell, vector<Edge> DomainEdge
//**************************************************************************

void TrianglePhysicalGeometryData::ConstructEdges() {

	// Внутренние переменные функции InitialiseGeometry():
	int number_counter;
	int i, j, n, m, k;
	int TypeOfPoint;
	
	bool FirstEdge;
	bool SecondEdge;
	bool ThirdEdge;
	
	// !?!?! Временная переменная
	double TEMP;
	

	/*
	// ВРЕМЕННАЯ ЗАПЛТАКА: Временное использую сетку, которая будет строится делением прямоугольника пополам
	// ЭТОТ КОД будем использовать, когда будет нужно превратить прямоугольную сетку в треугольную
	
	for (i=0; i<(Nx-1); i++)
	{
		for (j=0; j<(Ny-1); j++)
		{
			if (S[i*Ny+j]!=EMPTY_AREA && S[(i+1)*Ny+j]!=EMPTY_AREA && S[(i+1)*Ny+j+1]!=EMPTY_AREA && S[i*Ny+j+1]!=EMPTY_AREA)
			{				
				// Triangle #1 (i,j) (i+1,j) (i+1,j+1)
				TempElement.setElement(Convert_points[i*Ny+j], Convert_points[(i+1)*Ny+j], Convert_points[(i+1)*Ny+j+1]);
				
				DomainCell.push_back(TempElement);
				
				// Triangle #2 (i,j) (i+1,j+1) (i,j+1)
				TempElement.setElement(Convert_points[i*Ny+j], Convert_points[(i+1)*Ny+j+1], Convert_points[i*Ny+j+1]);
				
				DomainCell.push_back(TempElement);
				
			}
		}
	}
	*/
	

	//**************************************************************************
	// Создаем и заполняем массив vector<Edge> DomainEdge с ребрами
	//**************************************************************************


	// ПОЛЯ Edges
	// 1 элемент - номер точки, которая лежит в основании ребра
	// 2 элемент - номер точки, которая находится на вершине ребра
	// 3 элемент - номер ячейки, которая лежит СЛЕВА от ребра
	// 4 элемент - номер ячейки, которая лежит СПРАВА от ребра

	// 6б - нужно создать массив со всеми ребрами

	/*
	for (i=0; i<Nx; i++)
	{
		for (j=0; j<Ny; j++)
		{
			// если точка не принадлежит верхней и правой границе
			// то ей соответствует два ребра 
			// 1-ое (i,j) - (i+1,j)
			// 2-ое (i,j) - (i,j+1)

			if (i!=Nx-1 && j!=Ny-1)
			{
				Type_p1 = S[i*Ny+j];
				Type_p2 = S[(i+1)*Ny+j];
				
				if (Type_p1!=EMPTY_AREA && Type_p2!=EMPTY_AREA && (Type_p1==COMPUTATION_AREA || Type_p2==COMPUTATION_AREA) )
				{
					// 1-ое ребро (i,j) - (i+1,j)
					TempEdge.setEdge(Convert_points[i*Ny+j], Convert_points[(i+1)*Ny+j]);
					DomainEdge.push_back(TempEdge);					
				}
				
				Type_p1 = S[i*Ny+j];
				Type_p2 = S[i*Ny+j+1];

				if (Type_p1!=EMPTY_AREA && Type_p2!=EMPTY_AREA && (Type_p1==COMPUTATION_AREA || Type_p2==COMPUTATION_AREA) )
				{
					// 2-ое ребро (i,j) - (i,j+1)
					TempEdge.setEdge( Convert_points[i*Ny+j], Convert_points[i*Ny+j+1]);
					DomainEdge.push_back(TempEdge);									
				}
			}
			if (i==Nx-1 && j!=Ny-1)
			{
				Type_p1 = S[i*Ny+j];
				Type_p2 = S[i*Ny+j+1];
				
				if (Type_p1!=EMPTY_AREA && Type_p2!=EMPTY_AREA && (Type_p1==COMPUTATION_AREA || Type_p2==COMPUTATION_AREA) )
				{
					// 2-ое ребро (i,j) - (i,j+1)
					TempEdge.setEdge( Convert_points[i*Ny+j], Convert_points[i*Ny+j+1]);
					DomainEdge.push_back(TempEdge);					
				}
			}
			if (j==Ny-1 && i!=Nx-1)
			{
				Type_p1 = S[i*Ny+j];
				Type_p2 = S[(i+1)*Ny+j];
				
				if (Type_p1!=EMPTY_AREA && Type_p2!=EMPTY_AREA && (Type_p1==COMPUTATION_AREA || Type_p2==COMPUTATION_AREA) )
				{
					// 1-ое ребро (i,j) - (i+1,j)
					TempEdge.setEdge( Convert_points[i*Ny+j], Convert_points[(i+1)*Ny+j]);
					DomainEdge.push_back(TempEdge);					
				}
			}

		}
	}



	// запускаем два цикла. Внешний цикл перебирает все ребра, а внутренний все ячейки

	for (n=0; n<Total_number_of_edges; n++)
	{		
		edge_1 = DomainEdge[n].getElement(BEGIN_NODE);
		edge_2 = DomainEdge[n].getElement(END_NODE);

		for (m=0; m<Total_number_of_cells; m++)
		{
			cell_1 = DomainCell[m].getNode(FIRST_NODE);
			cell_2 = DomainCell[m].getNode(SECOND_NODE);
			cell_3 = DomainCell[m].getNode(THIRD_NODE);
			cell_4 = DomainCell[m].getNode(FOURTH_NODE);
			
			// 1
			if (edge_1==cell_1 && edge_2==cell_2)
			{
				// эта ячейка лежит СЛЕВА от ребра
				DomainEdge[n].setLeftElement(m);
			}

			// 2
			if (edge_1==cell_2 && edge_2==cell_3)
			{
				// эта ячейка лежит СЛЕВА от ребра
				DomainEdge[n].setLeftElement(m);
			}

			// 3
			if (edge_1==cell_3 && edge_2==cell_4)
			{
				// эта ячейка лежит СЛЕВА от ребра
				DomainEdge[n].setLeftElement(m);
			}

			// 4
			if (edge_1==cell_4 && edge_2==cell_1)
			{
				// эта ячейка лежит СЛЕВА от ребра
				DomainEdge[n].setLeftElement(m);
			}

			//==========================================

			// 1
			if (edge_1==cell_2 && edge_2==cell_1)
			{
				// эта ячейка лежит СПРАВА от ребра
				DomainEdge[n].setRightElement(m);
			}

			// 2
			if (edge_1==cell_3 && edge_2==cell_2)
			{
				// эта ячейка лежит СПРАВА от ребра
				DomainEdge[n].setRightElement(m);
			}

			// 3
			if (edge_1==cell_4 && edge_2==cell_3)
			{
				// эта ячейка лежит СПРАВА от ребра
				DomainEdge[n].setRightElement(m);
			}

			// 4
			if (edge_1==cell_1 && edge_2==cell_4)
			{
				// эта ячейка лежит СПРАВА от ребра
				DomainEdge[n].setRightElement(m);
			}

			//=========================================
		}
	}
	*/
	
	
	//**************************************************************************
	// НУЖНО переписать весь алгоритма для ребер
	//**************************************************************************

	for (m=0; m<Total_number_of_cells; m++)
	{
		// Предполагаем, что наша сетка треугольная
		cell_1 = DomainCell[m].getNode(FIRST_NODE);
		cell_2 = DomainCell[m].getNode(SECOND_NODE);
		cell_3 = DomainCell[m].getNode(THIRD_NODE);
					
		FirstEdge = false;	// Рассматриваемая сторона треугольника (cell_1 - cell_2) не принадлежит массиву ребер
		SecondEdge = false;
		ThirdEdge = false;
			
		if (DomainEdge.size() != 0)
		{
			for (n=0; n<DomainEdge.size(); n++)
			{
				edge_1 = DomainEdge[n].getElement(BEGIN_NODE);
				edge_2 = DomainEdge[n].getElement(END_NODE);
		
				// Проверка условия, что в DomainEdge[] уже лежит ребро (cell_1 - cell_2)
				// Каждое ребро принадлежит двум треугольникам (???) исключения в виде граничных ребер
				// Мы вносим в массив DomainEdge[] только одно ребро. 
				// Данное условие предотвращает повторное занесение в DomainEdge[] одного и того же ребра
						
				if (edge_1 == cell_2 && edge_2 == cell_1)
				{					
					FirstEdge = true;	//  FirstEdge
					DomainEdge[n].setRightElement(m);					
				}
				
				if (edge_1 == cell_3 && edge_2 == cell_2)
				{
					SecondEdge = true;
					DomainEdge[n].setRightElement(m);					
				}
				
				if (edge_1 == cell_1 && edge_2 == cell_3)
				{
					ThirdEdge = true;
					DomainEdge[n].setRightElement(m);					
				}
			}
		
		}
	
		if (FirstEdge != true)
		{
			if (Domain[cell_1].getType() == COMPUTATION_AREA || Domain[cell_2].getType() == COMPUTATION_AREA)
			{
				// Добавить ребро (cell_1 - cell_2) в массив ребер			
				TempEdge.setEdge( cell_1, cell_2, m);
				DomainEdge.push_back(TempEdge);
			}
		}
		if (SecondEdge != true)
		{
			if (Domain[cell_2].getType() == COMPUTATION_AREA || Domain[cell_3].getType() == COMPUTATION_AREA)
			{
				// Добавить ребро (cell_2 - cell_3) в массив ребер			
				TempEdge.setEdge( cell_2, cell_3, m);
				DomainEdge.push_back(TempEdge);
			}
		}
		if (ThirdEdge != true)
		{
			if (Domain[cell_3].getType() == COMPUTATION_AREA || Domain[cell_1].getType() == COMPUTATION_AREA)
			{
				// Добавить ребро (cell_2 - cell_3) в массив ребер			
				TempEdge.setEdge( cell_3, cell_1, m);
				DomainEdge.push_back(TempEdge);
			}
		}		
	}
	
	// ВАЖНО: Запишем общее число ребер
	Total_number_of_edges = DomainEdge.size();
	
	//**************************************************************************
	// Подсчитаем число соседей. Нам число соседей интересно только для граничных точек.
	// uptade: будем также хранить число соседей для внутренней точки (соседи граничные и внутренние)
	// !! В качестве соседей считаем только точки, принадлежащие расчетной области
	//**************************************************************************
	
	for (n=0; n<Total_number_of_edges; n++)
	{		
		edge_1 = DomainEdge[n].getElement(BEGIN_NODE);
		edge_2 = DomainEdge[n].getElement(END_NODE);
		
		Type_p1 = Domain[edge_1].getType();
		Type_p2 = Domain[edge_2].getType();

		if (Type_p1 != EMPTY_AREA)
		{
			if (Type_p2 == COMPUTATION_AREA && Type_p1 != COMPUTATION_AREA)
			{
				TypeOfPoint = Domain[edge_1].getNeighbour();
				Domain[edge_1].setNeighbour(TypeOfPoint+1);	// TypeOfPoint временная переменная
			}
			if (Type_p2 != EMPTY_AREA && Type_p1 == COMPUTATION_AREA)
			{
				TypeOfPoint = Domain[edge_1].getNeighbour();
				Domain[edge_1].setNeighbour(TypeOfPoint+1);
			}
		}
		
		if (Type_p2 != EMPTY_AREA)
		{
			if (Type_p1 == COMPUTATION_AREA && Type_p2 != COMPUTATION_AREA)
			{
				TypeOfPoint = Domain[edge_2].getNeighbour();
				Domain[edge_2].setNeighbour(TypeOfPoint+1);
			}
			if (Type_p1 != EMPTY_AREA && Type_p2 == COMPUTATION_AREA)
			{
				TypeOfPoint = Domain[edge_2].getNeighbour();
				Domain[edge_2].setNeighbour(TypeOfPoint+1);
			}
		}
	}
	
	// Значение CORNER_POINT присваивается точке, которая не соседствует с точками из расчетной
	// зоны. Но тем не менее эта точка принадлежит какому-либо расчетному шаблона. Поэтому на следующем
	// временном слое ее значения нужно задавать.
	
	for (k=0; k<Total_number_of_points; k++)
	{
		// условие 1) точка граничная 2) точка имеет 0 соседей с точками из расчетной области
		if(Domain[k].getType() != EMPTY_AREA && Domain[k].getType() != COMPUTATION_AREA && Domain[k].getNeighbour() == 0)
		{
			Domain[k].setType(CORNER_POINT);
			Domain[k].setNeighbour(2);	// В качестве двух соседей записываем другие граничные точки
		}
	}
	
	//**************************************************************************
	// Заканчиваем работу функции InitialiseGeometry(). Уничтожаем лишний массив
	//**************************************************************************
		
}

//**************************************************************************
// ExportGeometry() -> Записываем в текстовый файл данные о геометрии области
//**************************************************************************

void TrianglePhysicalGeometryData::ExportGeometry(const char* file_name) {

	FILE* f_id_export;
	char buffer_name[500];
	int N_smb;

	int k;
	
	//**************************************************************************
	// (этот код нужен только для тестирования)	Вывод информации о геометрии в текстовый файл
	//**************************************************************************

	f_id_export = fopen(file_name, "w");
	
	//************** массив Points
	N_smb = sprintf( buffer_name,"\n\nPoints  Total_number_of_points=%d\n", Total_number_of_points);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_export);
	
	for (k=0; k<Total_number_of_points; k++)
	{
		N_smb = sprintf( buffer_name,"%d\t%f\t%f\t%d\t%d\n", k, Domain[k].getX(), Domain[k].getY(), Domain[k].getType(), Domain[k-1].getNeighbour());
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_export);		
	}

	//************** массив Cells
	N_smb = sprintf( buffer_name,"\n\nCells  Total_number_of_cells=%d\n", Total_number_of_cells);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_export);
	
	for (k=0; k<Total_number_of_cells; k++)
	{
		N_smb = sprintf( buffer_name,"%d\t%d\t%d\t%d\n", k, DomainCell[k].getNode(FIRST_NODE), DomainCell[k].getNode(SECOND_NODE), DomainCell[k].getNode(THIRD_NODE));
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_export);		
	}


	//************** массив Edges
	N_smb = sprintf( buffer_name,"\n\nEdges  Total_number_of_edges=%d\n", Total_number_of_edges);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_export);
	
	for (k=0; k<Total_number_of_edges; k++)
	{
		N_smb = sprintf( buffer_name,"%d\t%d\t%d\t%d\t%d\n", k, DomainEdge[k].getElement(BEGIN_NODE), DomainEdge[k].getElement(END_NODE), DomainEdge[k].getElement(LEFT_ELEMENT), DomainEdge[k].getElement(RIGHT_ELEMENT));
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_export);				
	}

	fclose(f_id_export);

}

//**************************************************************************
// ComputeAverageLength() -> Расчет характерной длины расчетной ячейки
//**************************************************************************

void TrianglePhysicalGeometryData::ComputeAverageLength() {

	// Внутренние переменные функции ComputeAverageLength():
	int n;
	double S1, S2;
	
	//**************************************************************************
	//	Поиск величины усредненного ребра
	//**************************************************************************

	L_average = 0.0;

	for (n=0; n<Total_number_of_edges; n++)
	{
		Index_M0 = DomainEdge[n].getElement(BEGIN_NODE);
        Index_M2 = DomainEdge[n].getElement(END_NODE);
        Index_P1 = DomainEdge[n].getElement(RIGHT_ELEMENT); // номер ячейки/треугольника СПРАВА от ребра
        Index_P2 = DomainEdge[n].getElement(LEFT_ELEMENT); // номер ячейки/треугольника СЛЕВА от ребра

		if (Domain[Index_M0].getType() == COMPUTATION_AREA || Domain[Index_M2].getType() == COMPUTATION_AREA)
		{			
			TempElement = DomainCell[Index_P1];

			Index_p1_T1 = TempElement.getNode(FIRST_NODE); // точка номер 1 ячейки T1, находящейся СПРАВА от ребра
			Index_p2_T1 = TempElement.getNode(SECOND_NODE); // точка номер 2 ячейки T1, находящейся СПРАВА от ребра
			Index_p3_T1 = TempElement.getNode(THIRD_NODE); // точка номер 3 ячейки T1, находящейся СПРАВА от ребра
			
			TempElement = DomainCell[Index_P2];
			
			Index_p1_T2 = TempElement.getNode(FIRST_NODE); // точка номер 1 ячейки T2, находящейся СЛЕВА от ребра
			Index_p2_T2 = TempElement.getNode(SECOND_NODE); // точка номер 2 ячейки T2, находящейся СЛЕВА от ребра
			Index_p3_T2 = TempElement.getNode(THIRD_NODE); // точка номер 3 ячейки T2, находящейся СЛЕВА от ребра
			
			// Вычисляем координаты центров ячеек
		
			x_P1 = ( Domain[Index_p1_T1].getX() + Domain[Index_p2_T1].getX() + Domain[Index_p3_T1].getX() )/3.0;
			//
			x_P2 = ( Domain[Index_p1_T2].getX() + Domain[Index_p2_T2].getX() + Domain[Index_p3_T2].getX() )/3.0;
			//
			y_P1 = ( Domain[Index_p1_T1].getY() + Domain[Index_p2_T1].getY() + Domain[Index_p3_T1].getY() )/3.0;
			//
			y_P2 = ( Domain[Index_p1_T2].getY() + Domain[Index_p2_T2].getY() + Domain[Index_p3_T2].getY() )/3.0;
			
			
			L_P1P2 = sqrt( (x_P2-x_P1)*(x_P2-x_P1) + (y_P2-y_P1)*(y_P2-y_P1) );

			L_average = L_average + L_P1P2;
			
			Domain[Index_M0].addPerimeter(L_P1P2);
			Domain[Index_M2].addPerimeter(L_P1P2);
			
			// Добавим площадь для контрольного объема
			
			// х координата точек
			x_M0 = Domain[Index_M0].getX();
			x_M2 = Domain[Index_M2].getX();
			
			// y координата точек
			y_M0 = Domain[Index_M0].getY();
			y_M2 = Domain[Index_M2].getY();
			
			// (S1) 1 - M0, 2 - P1, 3 - P2
			//S = 0.5*abs( (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) ); 
			
			S1 = 0.5*fabs( (x_P1 - x_M0)*(y_P2 - y_M0) - (x_P2 - x_M0)*(y_P1 - y_M0) ); 
						
			// (S2) 1 - M2, 2 - P2, 3 - P1
			
			S2 = 0.5*fabs( (x_P2 - x_M2)*(y_P1 - y_M2) - (x_P1 - x_M2)*(y_P2 - y_M2) );
			
			Domain[Index_M0].addControlVolume(S1);
			Domain[Index_M2].addControlVolume(S2);

		}
		// END if (Domain[Index_M0].getType() == COMPUTATION_AREA || Domain[Index_M2].getType() == COMPUTATION_AREA)
	}

	L_average = L_average/Total_number_of_edges;
	
	// (???) Мне для удобства нужно заранее задать L_average
	//L_average = 0.5*(0.5+0.5);

}

//**************************************************************************
// maxH() -> Находит и возвращает максимальное значение H
//**************************************************************************

double TrianglePhysicalGeometryData::maxH() {

	int k;
	
	MAX_H = 0.0;
	for (k=0; k<Total_number_of_points; k++)
	{
		if (MAX_H < H[k])	MAX_H = H[k];			
	}
		
	return MAX_H;
}

//**************************************************************************
// ComputedNextTimeLayer() -> При заданном dT находим значения на следующем слое
//**************************************************************************

int TrianglePhysicalGeometryData::ComputeNextTimeLayer(double dT) {

	// Внутренние переменные функции ComputeNextTimeLayer():
	int n, k;

	double TEMP;
	int TypeOfPoint;
	int Number_of_Neighbours;
	
	#ifdef TEMP_CODE
	for (n=0; n<Total_number_of_points; n++)
	{
		if (n==42)
		{
			cout << "\nBEFORE CIRCLE LOOKING at tau[]" << endl;
			cout << "tau[n]\t" << tau[n] << endl;
			cin >> TEMP;
		}
	}
	#endif

	for (n=0; n<Total_number_of_edges; n++)
	{
		Index_M0 = DomainEdge[n].getElement(BEGIN_NODE);
		Index_M2 = DomainEdge[n].getElement(END_NODE);
		Index_P1 = DomainEdge[n].getElement(RIGHT_ELEMENT); // номер ячейки/треугольника СПРАВА от ребра
		Index_P2 = DomainEdge[n].getElement(LEFT_ELEMENT); // номер ячейки/треугольника СЛЕВА от ребра
            
		// Массив Edges разбит на блоки, по четыре элемента в каждом.
		// Первые два элемента являются номерами вершин, оставшиеся два элемента
		// являются номерами треугольников/ячеек. Далее дана расшифровка.
             
		// Index_M0	M0 - точка, которая является ОСНОВАНИЕМ РЕБРА
		// Index_M2	M2 - точка, которая является КОНЦОМ РЕБРА
		// Index_P1	P1 - номер треугольника/ячейки, находящемуся СПРАВА от ребра
		// Index_P2	P2 - номер треугольника/ячейки, находящемуся СЛЕВА от ребра
			 
		//**************************************************************************
		//	
		//**************************************************************************

							 
		if (Domain[Index_M0].getType() == COMPUTATION_AREA || Domain[Index_M2].getType() == COMPUTATION_AREA)
		{
			//**************************************************************************
			//	
			//**************************************************************************
			
			TempElement = DomainCell[Index_P1];

			Index_p1_T1 = TempElement.getNode(FIRST_NODE); // точка номер 1 ячейки T1, находящейся СПРАВА от ребра
			Index_p2_T1 = TempElement.getNode(SECOND_NODE); // точка номер 2 ячейки T1, находящейся СПРАВА от ребра
			Index_p3_T1 = TempElement.getNode(THIRD_NODE); // точка номер 3 ячейки T1, находящейся СПРАВА от ребра

			TempElement = DomainCell[Index_P2];
			
			Index_p1_T2 = TempElement.getNode(FIRST_NODE); // точка номер 1 ячейки T2, находящейся СЛЕВА от ребра
			Index_p2_T2 = TempElement.getNode(SECOND_NODE); // точка номер 2 ячейки T2, находящейся СЛЕВА от ребра
			Index_p3_T2 = TempElement.getNode(THIRD_NODE); // точка номер 3 ячейки T2, находящейся СЛЕВА от ребра
			
			//
			// х координата точек
			x_M0 = Domain[Index_M0].getX();
			x_M2 = Domain[Index_M2].getX();
			//
			 
			//
			x_P1 = ( Domain[Index_p1_T1].getX() + Domain[Index_p2_T1].getX() + Domain[Index_p3_T1].getX() )/3.0;
			//
			x_P2 = ( Domain[Index_p1_T2].getX() + Domain[Index_p2_T2].getX() + Domain[Index_p3_T2].getX() )/3.0;
								
			//
			// y координата точек
			y_M0 = Domain[Index_M0].getY();
			y_M2 = Domain[Index_M2].getY();
			//
			y_P1 = ( Domain[Index_p1_T1].getY() + Domain[Index_p2_T1].getY() + Domain[Index_p3_T1].getY() )/3.0;
			//
			y_P2 = ( Domain[Index_p1_T2].getY() + Domain[Index_p2_T2].getY() + Domain[Index_p3_T2].getY() )/3.0;
			//
             		 
			// значение H толщины слоя жидкости для точек
			H_M0 = H[Index_M0];
			H_M2 = H[Index_M2];
			//
			H_P1 = ( H[Index_p1_T1] + H[Index_p2_T1] + H[Index_p3_T1] )/3.0;
			//
			H_P2 = ( H[Index_p1_T2] + H[Index_p2_T2] + H[Index_p3_T2] )/3.0;
			//
			// значение xU для точек
			xU_M0 = xU[Index_M0];
			xU_M2 = xU[Index_M2];
			//
			xU_P1 = ( xU[Index_p1_T1] + xU[Index_p2_T1] + xU[Index_p3_T1] )/3.0;
			//
			xU_P2 = ( xU[Index_p1_T2] + xU[Index_p2_T2] + xU[Index_p3_T2] )/3.0;
			//
			// значение yU для точек             
			yU_M0 = yU[Index_M0];
			yU_M2 = yU[Index_M2];
			//
			yU_P1 = ( yU[Index_p1_T1] + yU[Index_p2_T1] + yU[Index_p3_T1] )/3.0;
			//
			yU_P2 = ( yU[Index_p1_T2] + yU[Index_p2_T2] + yU[Index_p3_T2] )/3.0;
			//
			// значение B для точек, величины описывающая поверхность дна
			B_M0 = B[Index_M0];
			B_M2 = B[Index_M2];
			//
			B_P1 = ( B[Index_p1_T1] + B[Index_p2_T1] + B[Index_p3_T1] )/3.0;
			//
			B_P2 = ( B[Index_p1_T2] + B[Index_p2_T2] + B[Index_p3_T2] )/3.0;
			//
			// значение ForceX для двух точек P1 и P2
			ForceX_P1 = ( ForceX[Index_p1_T1] + ForceX[Index_p2_T1] + ForceX[Index_p3_T1] )/3.0;
			//
			ForceX_P2 = ( ForceX[Index_p1_T2] + ForceX[Index_p2_T2] + ForceX[Index_p3_T2] )/3.0;
			//
			// значение ForceY для двух точек P1 и P2
			ForceY_P1 = ( ForceY[Index_p1_T1] + ForceY[Index_p2_T1] + ForceY[Index_p3_T1] )/3.0;
			//
			ForceY_P2 = ( ForceY[Index_p1_T2] + ForceY[Index_p2_T2] + ForceY[Index_p3_T2] )/3.0;
			//
			
					
			// значение площади, которое привязано к данному ребру
			S_edge = 0.5*fabs( (x_M0-x_M2)*(y_P1-y_P2) + (x_P1-x_P2)*(y_M2-y_M0) );
			//
			// Длина ребра, компоненты нормали к ребру, значения величин H,xU,yU на ребре
			//
			L_P1P2 = sqrt( (x_P2-x_P1)*(x_P2-x_P1) + (y_P2-y_P1)*(y_P2-y_P1) );
			nx_P32 = (y_P2-y_P1)/L_P1P2;
			ny_P32 = (x_P1-x_P2)/L_P1P2;             
			//
			H_P32 = 0.5*( H_P1 + H_P2 );
			xU_P32 = 0.5*( xU_P1 + xU_P2 );
			yU_P32 = 0.5*( yU_P1 + yU_P2 );
			B_P32 = 0.5*( B_P1 + B_P2 );
			//			
			//tau_P32 = (alpha*L_P1P2)/sqrt(g*H_P32);
			
			// !!!Специально для сухого дна
			tau_P32 = 0.5*(tau[Index_M0] + tau[Index_M2]);
			
			#ifdef TEMP_CODE
			if (Index_M0==42 || Index_M2==42)
			{
				cout << "\nLOOKING at tau_P32" << endl;
				cout << "Index_M0\t" << Index_M0 << endl;
				cout << "Index_M2\t" << Index_M2 << endl;
				
				cout << "tau[Index_M0]\t" << tau[Index_M0] << endl;
				cout << "tau[Index_M2]\t" << tau[Index_M2] << endl;				
			}
			#endif
						
			//
			ForceX_P32 = 0.5*( ForceX_P1 + ForceX_P2 );
			ForceY_P32 = 0.5*( ForceY_P1 + ForceY_P2 );
            
			//xJ_P32                                                                                     //                                                                      
			//xJ_P32 = H_P32*xU_P32 - tau_P32*( (( ( H_P2*xU_P2*xU_P2 - H_P1*xU_P1*xU_P1 )*( y_M0 - y_M2 ) 
			//	 + ( H_M2*xU_M2*xU_M2 - H_M0*xU_M0*xU_M0 )*( y_P2 - y_P1 ) )/(2*S_edge))...
			//	...
			//	 + (( ( H_P2*xU_P2*yU_P2 - H_P1*xU_P1*yU_P1 )*( x_M2 - x_M0 )... 
			//	 + ( H_M2*xU_M2*yU_M2 - H_M0*xU_M0*yU_M0 )*( x_P1 - x_P2 ) )/(2*S_edge))...
			//	...
			//	 + 0.5*g*(( ( H_P2*H_P2 - H_P1*H_P1 )*( y_M0 - y_M2 )... 
			//	 + ( H_M2*H_M2 - H_M0*H_M0 )*( y_P2 - y_P1 ) )/(2*S_edge))...
			//	...
			//	 + g*H_P32*(( ( B_P2 - B_P1 )*( y_M0 - y_M2 )... 
			//	 + ( B_M2 - B_M0 )*( y_P2 - y_P1 ) )/(2*S_edge))...
			//	...
			//	 - H_P32*ForceX_P32 );
			 
																				//
			TEMP = (( ( H_P2*xU_P2*xU_P2 - H_P1*xU_P1*xU_P1 )*( y_M0 - y_M2 ) + ( H_M2*xU_M2*xU_M2 - H_M0*xU_M0*xU_M0 )*( y_P2 - y_P1 ) )/(2*S_edge));
																				//
			TEMP += (( ( H_P2*xU_P2*yU_P2 - H_P1*xU_P1*yU_P1 )*( x_M2 - x_M0 ) + ( H_M2*xU_M2*yU_M2 - H_M0*xU_M0*yU_M0 )*( x_P1 - x_P2 ) )/(2*S_edge));
																		//
			TEMP += 0.5*g*(( ( H_P2*H_P2 - H_P1*H_P1 )*( y_M0 - y_M2 ) + ( H_M2*H_M2 - H_M0*H_M0 )*( y_P2 - y_P1 ) )/(2*S_edge));
																//
			TEMP += g*H_P32*(( ( B_P2 - B_P1 )*( y_M0 - y_M2 ) + ( B_M2 - B_M0 )*( y_P2 - y_P1 ) )/(2*S_edge));
			 
			TEMP -= H_P32*ForceX_P32;

			xJ_P32 = H_P32*xU_P32 - tau_P32*TEMP;
			
			
			#ifdef TEMP_CODE
			if (Index_M0==42 || Index_M2==42)
			{
				cout << "\nLOOKING at xJ_P32" << endl;
				cout << "Index_M0\t" << Index_M0 << endl;
				cout << "Index_M2\t" << Index_M2 << endl;
				
				cout << "\n" << endl;
				cout << "xJ_P32\t" << H_P32*xU_P32 - tau_P32*TEMP << endl;
				cout << "H_P32\t" << H_P32 << endl;
				cout << "xU_P32\t" << xU_P32 << endl;
				cout << "tau_P32\t" << tau_P32 << endl;
				cout << "TEMP\t" << TEMP << endl;
			}
			#endif
			
						 
			//yJ_P32
			//yJ_P32 = H_P32*yU_P32 - tau_P32*( (( ( H_P2*xU_P2*yU_P2 - H_P1*xU_P1*yU_P1 )*( y_M0 - y_M2 )...
			//    +( H_M2*xU_M2*yU_M2 - H_M0*xU_M0*yU_M0 )*( y_P2 - y_P1 ) )/(2*S_edge))...
			//    ...
			//    +(( ( H_P2*yU_P2*yU_P2 - H_P1*yU_P1*yU_P1 )*( x_M2 - x_M0 )...
			//    +( H_M2*yU_M2*yU_M2 - H_M0*yU_M0*yU_M0 )*( x_P1 - x_P2 ) )/(2*S_edge))...
			//    ...
			//    +0.5*g*(( ( H_P2*H_P2 - H_P1*H_P1 )*( x_M2 - x_M0 )...
			//    +( H_M2*H_M2 - H_M0*H_M0 )*( x_P1 - x_P2 ) )/(2*S_edge))...
			//    ...
			//    + g*H_P32*(( ( B_P2 - B_P1 )*( x_M2 - x_M0 )...
			//    +( B_M2 - B_M0 )*( x_P1 - x_P2 ) )/(2*S_edge))...
			//    ...
			//    - H_P32*ForceY_P32 );
																				//
			TEMP = (( ( H_P2*xU_P2*yU_P2 - H_P1*xU_P1*yU_P1 )*( y_M0 - y_M2 ) + ( H_M2*xU_M2*yU_M2 - H_M0*xU_M0*yU_M0 )*( y_P2 - y_P1 ) )/(2*S_edge));
																				//
			TEMP += (( ( H_P2*yU_P2*yU_P2 - H_P1*yU_P1*yU_P1 )*( x_M2 - x_M0 ) + ( H_M2*yU_M2*yU_M2 - H_M0*yU_M0*yU_M0 )*( x_P1 - x_P2 ) )/(2*S_edge));
																		//
			TEMP += 0.5*g*(( ( H_P2*H_P2 - H_P1*H_P1 )*( x_M2 - x_M0 ) + ( H_M2*H_M2 - H_M0*H_M0 )*( x_P1 - x_P2 ) )/(2*S_edge));
																//
			TEMP +=  g*H_P32*(( ( B_P2 - B_P1 )*( x_M2 - x_M0 ) + ( B_M2 - B_M0 )*( x_P1 - x_P2 ) )/(2*S_edge));

			TEMP -= H_P32*ForceY_P32;

			yJ_P32 = H_P32*yU_P32 - tau_P32*TEMP;

			//Rs_P32
			//Rs_P32 = g*tau_P32*( 0.5*xU_P32*( (( ( H_P2*H_P2 - H_P1*H_P1 )*( y_M0 - y_M2 )...
			//    +( H_M2*H_M2 - H_M0*H_M0 )*( y_P2 - y_P1 ) )/(2*S_edge)) )...
			//    ...
			//    + 0.5*yU_P32*( (( ( H_P2*H_P2 - H_P1*H_P1 )*( x_M2 - x_M0 )...
			//    +( H_M2*H_M2 - H_M0*H_M0 )*( x_P1 - x_P2 ) )/(2*S_edge)) )...
			//    ...
			//    + H_P32*H_P32*( (( ( xU_P2 - xU_P1 )*( y_M0 - y_M2 )...
			//    +( xU_M2 - xU_M0 )*( y_P2 - y_P1 ) )/(2*S_edge))...
			//    ...
			//    + (( ( yU_P2 - yU_P1 )*( x_M2 - x_M0 )...
			//    +( yU_M2 - yU_M0 )*( x_P1 - x_P2 ) )/(2*S_edge)) ) );
			 
																			//
			TEMP = 0.5*xU_P32*( (( ( H_P2*H_P2 - H_P1*H_P1 )*( y_M0 - y_M2 ) + ( H_M2*H_M2 - H_M0*H_M0 )*( y_P2 - y_P1 ) )/(2*S_edge)) );
																				//
			TEMP += 0.5*yU_P32*( (( ( H_P2*H_P2 - H_P1*H_P1 )*( x_M2 - x_M0 ) + ( H_M2*H_M2 - H_M0*H_M0 )*( x_P1 - x_P2 ) )/(2*S_edge)) );
			//										//
			TEMP += H_P32*H_P32*( (( ( xU_P2 - xU_P1 )*( y_M0 - y_M2 ) + ( xU_M2 - xU_M0 )*( y_P2 - y_P1 ) )/(2*S_edge)) + (( ( yU_P2 - yU_P1 )*( x_M2 - x_M0 ) + ( yU_M2 - yU_M0 )*( x_P1 - x_P2 ) )/(2*S_edge)) );

			Rs_P32 = g*tau_P32*TEMP;
		
			//
			//xWs_P32
			//xWs_P32 = tau_P32*( H_P32*( xU_P32*(( ( xU_P2 - xU_P1 )*( y_M0 - y_M2 )...
			//    +( xU_M2 - xU_M0 )*( y_P2 - y_P1 ) )/(2*S_edge))...
			//    ...
			//    + yU_P32*(( ( xU_P2 - xU_P1 )*( x_M2 - x_M0 )...
			//    +( xU_M2 - xU_M0 )*( x_P1 - x_P2 ) )/(2*S_edge)) )...
			//    ...
			//    + 0.5*g*(( ( H_P2*H_P2 - H_P1*H_P1 )*( y_M0 - y_M2 )...
			//    +( H_M2*H_M2 - H_M0*H_M0 )*( y_P2 - y_P1 ) )/(2*S_edge))...
			//    ...
			//    + g*H_P32*(( ( B_P2 - B_P1 )*( y_M0 - y_M2 )...
			//    +( B_M2 - B_M0 )*( y_P2 - y_P1 ) )/(2*S_edge))...
			//    ...
			//    - H_P32*ForceX_P32 );
	
			TEMP = H_P32*xU_P32*(( ( xU_P2 - xU_P1 )*( y_M0 - y_M2 ) + ( xU_M2 - xU_M0 )*( y_P2 - y_P1 ) )/(2*S_edge)) + H_P32*yU_P32*(( ( xU_P2 - xU_P1 )*( x_M2 - x_M0 ) + ( xU_M2 - xU_M0 )*( x_P1 - x_P2 ) )/(2*S_edge));			 
																		//
			TEMP += 0.5*g*(( ( H_P2*H_P2 - H_P1*H_P1 )*( y_M0 - y_M2 ) + ( H_M2*H_M2 - H_M0*H_M0 )*( y_P2 - y_P1 ) )/(2*S_edge));
																//
			TEMP += g*H_P32*(( ( B_P2 - B_P1 )*( y_M0 - y_M2 ) + ( B_M2 - B_M0 )*( y_P2 - y_P1 ) )/(2*S_edge));

			TEMP -= H_P32*ForceX_P32;

			xWs_P32 = tau_P32*TEMP;

			//
			//yWs_P32
			//yWs_P32 = tau_P32*( H_P32*( xU_P32*(( ( yU_P2 - yU_P1 )*( y_M0 - y_M2 )...
			//    +( yU_M2 - yU_M0 )*( y_P2 - y_P1 ) )/(2*S_edge))...
			//    ...
			//    + yU_P32*(( ( yU_P2 - yU_P1 )*( x_M2 - x_M0 )...
			//    +( yU_M2 - yU_M0 )*( x_P1 - x_P2 ) )/(2*S_edge)) )...
			//    ...
			//    +0.5*g*(( ( H_P2*H_P2 - H_P1*H_P1 )*( x_M2 - x_M0 )...
			//    +( H_M2*H_M2 - H_M0*H_M0 )*( x_P1 - x_P2 ) )/(2*S_edge))...
			//    ...
			//    + g*H_P32*(( ( B_P2 - B_P1 )*( x_M2 - x_M0 )...
			//    +( B_M2 - B_M0 )*( x_P1 - x_P2 ) )/(2*S_edge))...
			//    ...
			//    - H_P32*ForceY_P32 );
			
			TEMP = H_P32*xU_P32*(( ( yU_P2 - yU_P1 )*( y_M0 - y_M2 ) + ( yU_M2 - yU_M0 )*( y_P2 - y_P1 ) )/(2*S_edge)) + H_P32*yU_P32*(( ( yU_P2 - yU_P1 )*( x_M2 - x_M0 ) + ( yU_M2 - yU_M0 )*( x_P1 - x_P2 ) )/(2*S_edge));
																		//
			TEMP += 0.5*g*(( ( H_P2*H_P2 - H_P1*H_P1 )*( x_M2 - x_M0 ) + ( H_M2*H_M2 - H_M0*H_M0 )*( x_P1 - x_P2 ) )/(2*S_edge));
																//
			TEMP += g*H_P32*(( ( B_P2 - B_P1 )*( x_M2 - x_M0 ) + ( B_M2 - B_M0 )*( x_P1 - x_P2 ) )/(2*S_edge));

			TEMP -= H_P32*ForceY_P32;

			yWs_P32 = tau_P32*TEMP;


			xxPT_P32 = xU_P32*xWs_P32 + Rs_P32;
			xyPT_P32 = xU_P32*yWs_P32;
			yxPT_P32 = yU_P32*xWs_P32;
			yyPT_P32 = yU_P32*yWs_P32 + Rs_P32;
					
								
			//Index_M0
			if (Domain[Index_M0].getType() == COMPUTATION_AREA)
			{
				Set_for_H_flow[Index_M0] = Set_for_H_flow[Index_M0] + (xJ_P32*nx_P32 + yJ_P32*ny_P32)*L_P1P2;
				//                                                                          //                                                                   //
				Set_for_H_ux_flow[Index_M0] = Set_for_H_ux_flow[Index_M0] + (xxPT_P32*nx_P32 - xU_P32*xJ_P32*nx_P32 - 0.5*g*H_P32*H_P32*nx_P32 + yxPT_P32*ny_P32 - xU_P32*yJ_P32*ny_P32)*L_P1P2;
				//                                                                          //                                         //
				Set_for_H_uy_flow[Index_M0] = Set_for_H_uy_flow[Index_M0] + (xyPT_P32*nx_P32 - yU_P32*xJ_P32*nx_P32 + yyPT_P32*ny_P32 - yU_P32*yJ_P32*ny_P32 - 0.5*g*H_P32*H_P32*ny_P32)*L_P1P2;
				//
				Set_for_B_dx[Index_M0] = Set_for_B_dx[Index_M0] + (g*B_P32*nx_P32)*L_P1P2;
				//
				Set_for_B_dy[Index_M0] = Set_for_B_dy[Index_M0] + (g*B_P32*ny_P32)*L_P1P2;
				//
				Set_for_H_ux_uy[Index_M0] = Set_for_H_ux_uy[Index_M0] + (H_P32*xU_P32*nx_P32 + H_P32*yU_P32*ny_P32)*L_P1P2;
				//
			}
			//Index_M2
			if (Domain[Index_M2].getType() == COMPUTATION_AREA)
			{
				//
				Set_for_H_flow[Index_M2] = Set_for_H_flow[Index_M2] - (xJ_P32*nx_P32 + yJ_P32*ny_P32)*L_P1P2;
				//                                                                            //                                                                   //
				Set_for_H_ux_flow[Index_M2] = Set_for_H_ux_flow[Index_M2] - (xxPT_P32*nx_P32 - xU_P32*xJ_P32*nx_P32 - 0.5*g*H_P32*H_P32*nx_P32 + yxPT_P32*ny_P32 - xU_P32*yJ_P32*ny_P32)*L_P1P2;
				//                                                                            //                                        //
				Set_for_H_uy_flow[Index_M2] = Set_for_H_uy_flow[Index_M2] - (xyPT_P32*nx_P32 - yU_P32*xJ_P32*nx_P32 + yyPT_P32*ny_P32 - yU_P32*yJ_P32*ny_P32 - 0.5*g*H_P32*H_P32*ny_P32)*L_P1P2;
				//
				Set_for_B_dx[Index_M2] = Set_for_B_dx[Index_M2] - (g*B_P32*nx_P32)*L_P1P2;
				//
				Set_for_B_dy[Index_M2] = Set_for_B_dy[Index_M2] - (g*B_P32*ny_P32)*L_P1P2;
				//
				Set_for_H_ux_uy[Index_M2] = Set_for_H_ux_uy[Index_M2] - (H_P32*xU_P32*nx_P32 + H_P32*yU_P32*ny_P32)*L_P1P2;
				//
			}             
			//		

			
			#ifdef TEMP_CODE
			if (Index_M0==42 || Index_M2==42)
			{
				cout << "\nLOOKING at Set_for_H_flow[]" << endl;
				cout << "Index_M0\t" << Index_M0 << endl;
				cout << "Index_M2\t" << Index_M2 << endl;
				cout << "Set_for_H_flow[Index_M0]\t" << Set_for_H_flow[Index_M0] << endl;
				cout << "Set_for_H_flow[Index_M2]\t" << Set_for_H_flow[Index_M2] << endl;
				
				cout << "\n" << endl;
				cout << "xJ_P32\t" << xJ_P32 << endl;
				cout << "nx_P32\t" << nx_P32 << endl;
				cout << "yJ_P32\t" << yJ_P32 << endl;
				cout << "ny_P32\t" << ny_P32 << endl;
				cout << "L_P1P2\t" << L_P1P2 << endl;
			}			
			#endif
			
			

		}
		// END  if (Domain[Index_M0].getType() == COMPUTATION_AREA || Domain[Index_M2].getType() == COMPUTATION_AREA)

	}
	// END for (n=0; n<Total_number_of_edges; n++)


	//**************************************************************************
	//	Вычисляем значения на следующем временном слое
	//**************************************************************************

	//
	for (k=0; k<Total_number_of_points; k++)
	{
		if (Domain[k].getType() == COMPUTATION_AREA)
		{
			//
			TEMP = H[k];
					
			S_control = Domain[k].getControlVolume();
					
			H[k] = H[k] - (dT/S_control)*Set_for_H_flow[k];

			//
			if ( H[k]>epsilon[k] )
			{
				xU[k] = ( TEMP*xU[k] + (dT/S_control)*Set_for_H_ux_flow[k] + dT*(ForceX[k] - (Set_for_B_dx[k]/S_control) )*(TEMP - (tau[k]/S_control)*Set_for_H_ux_uy[k]) )/H[k];
				//
				yU[k] = ( TEMP*yU[k] + (dT/S_control)*Set_for_H_uy_flow[k] + dT*(ForceY[k] - (Set_for_B_dy[k]/S_control) )*(TEMP - (tau[k]/S_control)*Set_for_H_ux_uy[k]) )/H[k];
			}
			else
			{
				xU[k] = 0.0;
				yU[k] = 0.0;
			}
				
		}
	}
					
	// Если H<0 значит алгоритм разваливается и нужно вернуть ERROR_SHALLOW_WATER_COMPUTATION
	for (k=0; k<Total_number_of_points; k++)
	{
		if (Domain[k].getType() == COMPUTATION_AREA)
		{
			if (H[k] < 0.0)
			{
				cout << "\nError H<0" << endl;
				cout << "k\t" << k << endl;
				cout << "H[k]\t" << H[k] << endl;
				
				#ifdef TEMP_CODE
				cout << "(dT/S_control)\t" << (dT/S_control) << endl;
				cout << "Set_for_H_flow[k]\t" << Set_for_H_flow[k] << endl;
				#endif
				
				cin >> TEMP;
				return ERROR_SHALLOW_WATER_COMPUTATION;
			}			
		}
	}
		
	
	//**************************************************************************
	//	Обработка граничных точек
	//**************************************************************************
			
	// Подсчитаем суммы значений, взятых из всех соседних точек
	for (n=0; n<Total_number_of_edges; n++)
	{				
		Index_M0 = DomainEdge[n].getElement(BEGIN_NODE);
		Index_M2 = DomainEdge[n].getElement(END_NODE);
				
		Type_p1 = Domain[Index_M0].getType();
		Type_p2 = Domain[Index_M2].getType();
				
		// точка Index_M0
		if (Type_p1 != EMPTY_AREA && Type_p1 != COMPUTATION_AREA && Type_p1 != CORNER_POINT)
		{
			if (Type_p2 == COMPUTATION_AREA)
			{
				Set_for_H_flow[Index_M0] = Set_for_H_flow[Index_M0] + H[Index_M2] ;
				Set_for_H_ux_flow[Index_M0] = Set_for_H_ux_flow[Index_M0] + xU[Index_M2]; // Вообще-то Set_for_H_ux_flow предназначен для HUx, но будем записывать xU
				Set_for_H_uy_flow[Index_M0] = Set_for_H_uy_flow[Index_M0] + yU[Index_M2];	
			}
		}
				
		// точка Index_M2
		if (Type_p2 != EMPTY_AREA && Type_p2 != COMPUTATION_AREA && Type_p2 != CORNER_POINT)
		{
			if (Type_p1 == COMPUTATION_AREA)
			{
				Set_for_H_flow[Index_M2] = Set_for_H_flow[Index_M2] + H[Index_M0] ;
				Set_for_H_ux_flow[Index_M2] = Set_for_H_ux_flow[Index_M2] + xU[Index_M0]; // Вообще-то Set_for_H_ux_flow предназначен для HUx, но будем записывать xU
				Set_for_H_uy_flow[Index_M2] = Set_for_H_uy_flow[Index_M2] + yU[Index_M0];
			}
		}				
	}
			
	// Находим значения граничных точек
	for (k=0; k<Total_number_of_points; k++)
	{
		TypeOfPoint = Domain[k].getType();
		Number_of_Neighbours = Domain[k].getNeighbour();
								
		switch(TypeOfPoint)
		{
			case DOWN_WALL:
			H[k] = Set_for_H_flow[k]/Number_of_Neighbours;	
			xU[k] = 0.0;				
			yU[k] = 0.0;
			break;
					
			case UPPER_WALL:
			H[k] = Set_for_H_flow[k]/Number_of_Neighbours;	
			xU[k] = 0.0;				
			yU[k] = 0.0;
			break;
					
			case LEFT_WALL:
			H[k] = Set_for_H_flow[k]/Number_of_Neighbours;	
			xU[k] = 0.0;				
			yU[k] = 0.0;
			break;
			
			case RIGHT_WALL:
			H[k] = Set_for_H_flow[k]/Number_of_Neighbours;	
			xU[k] = 0.0;				
			yU[k] = 0.0;
			break;
			
			case DOWN_FREE:
			H[k] = Set_for_H_flow[k]/Number_of_Neighbours;	
			xU[k] = Set_for_H_ux_flow[k]/Number_of_Neighbours;				
			yU[k] = Set_for_H_uy_flow[k]/Number_of_Neighbours;
			break;	
			
			case UPPER_FREE:
			H[k] = Set_for_H_flow[k]/Number_of_Neighbours;	
			xU[k] = Set_for_H_ux_flow[k]/Number_of_Neighbours;				
			yU[k] = Set_for_H_uy_flow[k]/Number_of_Neighbours;
			break;	
					
			case LEFT_FREE:
			H[k] = Set_for_H_flow[k]/Number_of_Neighbours;	
			xU[k] = Set_for_H_ux_flow[k]/Number_of_Neighbours;				
			yU[k] = Set_for_H_uy_flow[k]/Number_of_Neighbours;
			break;	
			
			case RIGHT_FREE:
			H[k] = Set_for_H_flow[k]/Number_of_Neighbours;	
			xU[k] = Set_for_H_ux_flow[k]/Number_of_Neighbours;				
			yU[k] = Set_for_H_uy_flow[k]/Number_of_Neighbours;
			break;					
				
		}	
	}
			
	//**************************************************************************
	//	Короче, для CORNER_POINT придется писать отдельный код, потому что эта точка является исключением
	//**************************************************************************
			
	// Подсчитаем суммы значений, взятых из всех соседних точек
	// Считаем только для точек типа CORNER_POINT
	for (n=0; n<Total_number_of_edges; n++)
	{				
		Index_M0 = DomainEdge[n].getElement(BEGIN_NODE);
		Index_M2 = DomainEdge[n].getElement(END_NODE);
				
		Type_p1 = Domain[Index_M0].getType();
		Type_p2 = Domain[Index_M2].getType();
				
		// точка Index_M0
		if (Type_p1 == CORNER_POINT)
		{
			Set_for_H_flow[Index_M0] = Set_for_H_flow[Index_M0] + H[Index_M2] ;
			Set_for_H_ux_flow[Index_M0] = Set_for_H_ux_flow[Index_M0] + xU[Index_M2];
			Set_for_H_uy_flow[Index_M0] = Set_for_H_uy_flow[Index_M0] + yU[Index_M2];					
		}

		// точка Index_M2
		if (Type_p2 == CORNER_POINT)
		{
			Set_for_H_flow[Index_M2] = Set_for_H_flow[Index_M2] + H[Index_M0] ;
			Set_for_H_ux_flow[Index_M2] = Set_for_H_ux_flow[Index_M2] + xU[Index_M0];
			Set_for_H_uy_flow[Index_M2] = Set_for_H_uy_flow[Index_M2] + yU[Index_M0];
		}
			
				
	}
			
	// Находим значения граничных точек типа CORNER_POINT
	for (k=0; k<Total_number_of_points; k++)
	{							
		if (Domain[k].getType() == CORNER_POINT)
		{
			Number_of_Neighbours = Domain[k].getNeighbour();
					
			H[k] = Set_for_H_flow[k]/Number_of_Neighbours;	
			xU[k] = Set_for_H_ux_flow[k]/Number_of_Neighbours;				
			yU[k] = Set_for_H_uy_flow[k]/Number_of_Neighbours;
		}				
	}
	
	//**************************************************************************
	// Значения tau нужно найти для всех точек, а не только внутренних
	//**************************************************************************
	
	for (k=0; k<Total_number_of_points; k++)
	{
		if ( H[k] > epsilon[k] )
		{
			tau[k] = (alpha*(Domain[k].getPerimeter()/Domain[k].getNeighbour()))/sqrt(g*H[k]);
		}
		else
		{
			tau[k] = 0.0;
		}
	}
	
	//**************************************************************************
	// Массивы, в которых хранятся суммы потоков, нужно обнулить
	//**************************************************************************
	
	Set_for_H_flow.assign(Total_number_of_points, 0.0);
	Set_for_H_ux_flow.assign(Total_number_of_points, 0.0);
	Set_for_H_uy_flow.assign(Total_number_of_points, 0.0);
	
	Set_for_B_dx.assign(Total_number_of_points, 0.0);
	Set_for_B_dy.assign(Total_number_of_points, 0.0);
	Set_for_H_ux_uy.assign(Total_number_of_points, 0.0);
	
	#ifdef TEMP_CODE
	for (n=0; n<Total_number_of_points; n++)
	{
		if (n==42)
		{
			cout << "\nAFTER CIRCLE LOOKING at tau[]" << endl;
			cout << "tau[n]\t" << tau[n] << endl;
			cin >> TEMP;
		}
	}
	#endif
	
	// END of method TrianglePhysicalGeometryData::ComputedNextTimeLayer()
	return CONTINUE_SHALLOW_WATER_COMPUTATION;
}

//**************************
// Name: ImportFemlabPoints(const char* file_name)
// Description: Импортирует из бинарного файла информацию о координатах точек
//**************************

void TrianglePhysicalGeometryData::ImportFemlabPoints(const char* file_name)
{
	FILE* f_id;
	int N, M;	// Матлаб записывает матрицу [N M] Цикл по N должен быть внутренним
	int i, j;
	
	double TEMP_1, TEMP_2;

    // Qt
    QFile file_id(file_name);
    
    // Пишут, что в Qt разрешено использовать только /
    // в независимости от операционной системы
    
    file_id.open(QIODevice::ReadOnly);    
    QDataStream input_data(&file_id);
    
    // Открываем текстовый файл для записи в него информации
    
    QFile textconsole_id("C:/FILES/Matlab and C++/VisualStudio2010/qtMesh/ConsolePoints.txt");
    
    textconsole_id.open(QFile::WriteOnly | QFile::Text);
     
    QTextStream output_data(&textconsole_id);

    //int N;
    //int M;
    //double TEMP_1;
    //double TEMP_2;
    
    //fread(&TEMP_1, sizeof(double), 1, f_id);
	//fread(&TEMP_2, sizeof(double), 1, f_id);
    
    input_data.setByteOrder(QDataStream::LittleEndian);
    input_data >> TEMP_1 >> TEMP_2;
    
    
    //file_id.read( (char *) &TEMP_1, sizeof(TEMP_1) );
    //file_id.read( (char *) &TEMP_2, sizeof(TEMP_2) );
    //TEMP_1 = ntohl( TEMP_1 );
    //TEMP_2 = ntohl( TEMP_2 );
	
    output_data << TEMP_1 << "\t" << TEMP_2 << "\n";
    
	// Дело в том, что я записывал в бинарный файл double, поэтому пусть при извлечении тоже
	// записывает в double
	N = int(TEMP_1);
	M = int(TEMP_2);
    
    Total_number_of_points = M;
	Domain.reserve(Total_number_of_points);	// Сразу выделяем память для vector<Point> Domain
	
    
	for (j=0; j<M; j++)
	{
		//fread(&TEMP_1, sizeof(double), 1, f_id);
		//fread(&TEMP_2, sizeof(double), 1, f_id);
		
        input_data >> TEMP_1 >> TEMP_2;
            
		// В качестве третьего параметра указываю произвольное число, т.к. оно необходимо
			
		TempPoint.setValues(TEMP_1, TEMP_2, 18);
		Domain.push_back(TempPoint);
	}
    
    
    file_id.close();
    
    //fclose(f_id);
    
       
    for (int j=0; j<Total_number_of_points; j++) {
        // Обращаемся к точкам
        //Domain[cell_1].getX();
    
        output_data << Domain[j].getX() << "\t" << Domain[j].getY() << "\n";
    }
    
    
    textconsole_id.flush();
    textconsole_id.close();

	
}

//**************************
// Name: exportDataToTxtFile()
// Description: Запишем прочитанную информацию в текстовый файл
//**************************

void TrianglePhysicalGeometryData::exportDataToTxtFile() {

    QFile file_id("C:/FILES/Matlab and C++/VisualStudio2010/qtMesh/Console.txt");
    
    file_id.open(QFile::WriteOnly | QFile::Text);
     
    QTextStream output_data(&file_id);
    output_data << Total_number_of_points << "\n";
    
    for (int j=0; j<Total_number_of_points; j++) {
        // Обращаемся к точкам
        //Domain[cell_1].getX();
    
        output_data << Domain[j].getX() << "\t" << Domain[j].getY() << "\n";
    }
    
    file_id.flush();
    file_id.close();

}

//***********************
// Name: ImportFemlabTriangles(const char* file_name)
// Description: Импортирует из бинарного файла информацию о треугольниках сетки
// В отдельном файле хранится информация о точках. Мда, не самое лучшее решение.
//***********************


void TrianglePhysicalGeometryData::ImportFemlabTriangles(const char* file_name)
{
	FILE* f_id;
	int N, M;	
	int i, j;
	
	double TEMP_1, TEMP_2;
	double TEMP_3, TEMP_4;
	
	int i1, i2, i3;
	
	//f_id = fopen(file_name, "rb");	
	//fread(&TEMP_1, sizeof(double), 1, f_id);
	//fread(&TEMP_2, sizeof(double), 1, f_id);
	
    QFile file_id(file_name);
    file_id.open(QIODevice::ReadOnly);    
    QDataStream input_data(&file_id);
    
    // Set correct byte order to read "double"
    input_data.setByteOrder(QDataStream::LittleEndian);

    input_data >> TEMP_1 >> TEMP_2;
    
	// Дело в том, что я записывал в бинарный файл double, поэтому пусть при извлечении тоже
	// записывает в double
	N = int(TEMP_1);
	M = int(TEMP_2);
	
	Total_number_of_cells = M;
	DomainCell.reserve(Total_number_of_cells); // Выделяем память для для vector<SquareCell> DomainCell
	
	for (j=0; j<M; j++)
	{
		//fread(&TEMP_1, sizeof(double), 1, f_id);
		//fread(&TEMP_2, sizeof(double), 1, f_id);
		//fread(&TEMP_3, sizeof(double), 1, f_id);
		//fread(&TEMP_4, sizeof(double), 1, f_id);
		
        input_data >> TEMP_1 >> TEMP_2 >> TEMP_3 >> TEMP_4;
        
		i1 = int(TEMP_1) - 1;
		i2 = int(TEMP_2) - 1;
		i3 = int(TEMP_3) - 1;
			
		TempElement.setElement(i1, i2, i3);
				
		DomainCell.push_back(TempElement);

	}
	
	//fclose(f_id);
    
    file_id.close();
	
}

//**************************************************************************
// Name: ConfirmCounterClockwise()
// Description: Меняет номера точек так, чтобы номера точек треугольник шли против часовой стрелки
//**************************************************************************

void TrianglePhysicalGeometryData::ConfirmCounterClockwise()
{
	int m;
	double x1, y1, x2, y2, x3, y3;
	double sa;

	for (m=0; m<Total_number_of_cells; m++)
	{	

		cell_1 = DomainCell[m].getNode(FIRST_NODE);
		cell_2 = DomainCell[m].getNode(SECOND_NODE);
		cell_3 = DomainCell[m].getNode(THIRD_NODE);
		
		x1 = Domain[cell_1].getX();
		y1 = Domain[cell_1].getY();
		
		x2 = Domain[cell_2].getX();
		y2 = Domain[cell_2].getY();
		
		x3 = Domain[cell_3].getX();
		y3 = Domain[cell_3].getY();
		
		sa = (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);
		
		if ( sa < 0 )
		{
			// Меняем порядок последних элементов
			DomainCell[m].setElement(cell_1, cell_3, cell_2);
		}
	}
		
}

//**************************************************************************
// Name: Temp_InitialiseTypeOfPoints()
// Description: Функция в РУЧНОМ режиме присваивает типы точкам
//**************************************************************************

void TrianglePhysicalGeometryData::Temp_InitialiseTypeOfPoints(double Xmin, double Xmax, double Ymin, double Ymax)
{
	int n;
	
	double X, Y;
			
	double Delta = 0.01;

	for (n=0; n<Total_number_of_points; n++)
	{
		X = Domain[n].getX();
		Y = Domain[n].getY();
		
		Domain[n].setType(EMPTY_AREA);
		
		// Area -- COMPUTATION_AREA
		if (Y > Ymin && Y < Ymax)
		{
			if (X > Xmin && X < Xmax)
			{
				Domain[n].setType(COMPUTATION_AREA);
				
				#ifdef TEMP_CODE
				if (n==197)	
				{
					cout << "NOT Appropriate number" << endl;
					cout << "X\t" << Domain[n].getX() << endl;
					cout << "Y\t" << Domain[n].getY() << endl;
					cout << "Y > Ymin && Y < Ymax\t" << (Y > Ymin && Y < Ymax) << endl;
					cout << "Y >= (Ymax-Delta) && Y <= (Ymax+Delta)\t" << (Y >= (Ymax-Delta) && Y <= (Ymax+Delta)) << endl;
				}
				#endif
			}
		}
		
		// Boundary Area
		if (Y >= (Ymin-Delta) && Y <= (Ymin+Delta))
		{
			Domain[n].setType(DOWN_WALL);
		}
		if (Y >= (Ymax-Delta) && Y <= (Ymax+Delta))
		{
			Domain[n].setType(UPPER_WALL);
			
			#ifdef TEMP_CODE
			if (n==197)	
			{
				cout << "Appropriate number" << endl;
			}
			#endif
		}
		if (X >= (Xmin-Delta) && X <= (Xmin+Delta))
		{
			Domain[n].setType(LEFT_WALL);
		}
		if (X >= (Xmax-Delta) && X <= (Xmax+Delta))
		{
			Domain[n].setType(RIGHT_WALL);
		}
		
		
	}
}

//**************************************************************************
// Name: ExportGeometryToTecplot(const char* file_name)
// Description: Записывает в файл только сетку
//**************************************************************************

void TrianglePhysicalGeometryData::ExportGeometryToTecplot(const char* file_name)
{
	FILE* f_id_tecplot;
	char buffer_name[500];
	int N_smb;
	
	int k, m;	// Эти переменные нужны для циклов

	
	f_id_tecplot = fopen(file_name, "w");
			
	//VARIABLES = X,Y,S,N
	N_smb = sprintf( buffer_name,"VARIABLES = X,Y,S,N\n");	// Не забуть изменить "%f\t...\n"
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

	// ZONE T="Time 0.0", DATAPACKING=POINT, NODES=%d, ELEMENTS=%d, ZONETYPE=FETRIANGLE\n
	N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", DATAPACKING=POINT, NODES=%d, ELEMENTS=%d, ZONETYPE=FETRIANGLE\n", 0.0, Total_number_of_points, Total_number_of_cells);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	
	for (k=0; k<Total_number_of_points; k++)
	{
		N_smb = sprintf( buffer_name,"%f\t%f\t%d\t%d\n", Domain[k].getX(), Domain[k].getY(), Domain[k].getType(), Domain[k].getNeighbour() );
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);				
	}
								
	N_smb = sprintf( buffer_name,"\n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
				
	for (m=0; m<Total_number_of_cells; m++)
	{					
		N_smb = sprintf( buffer_name,"%d %d %d\n", DomainCell[m].getNode(FIRST_NODE)+1, DomainCell[m].getNode(SECOND_NODE)+1, DomainCell[m].getNode(THIRD_NODE)+1);
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	}

	fclose(f_id_tecplot);
}

//**************************************************************************
// Name: ExportEdgesToTecplot(const char* file_name)
// Description: Записывает в файл Tecplot только ребра
//**************************************************************************

void TrianglePhysicalGeometryData::ExportEdgesToTecplot(const char* file_name)
{
	FILE* f_id_tecplot;
	char buffer_name[500];
	int N_smb;
	
	int k, m;	// Эти переменные нужны для циклов

	
	f_id_tecplot = fopen(file_name, "w");
			
	//VARIABLES = X,Y,S
	N_smb = sprintf( buffer_name,"VARIABLES = X,Y,S\n");	// Не забуть изменить "%f\t...\n"
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

	// ZONE T="Time 0.0", DATAPACKING=POINT, NODES=%d, ELEMENTS=%d, ZONETYPE=FETRIANGLE\n
	N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", DATAPACKING=POINT, NODES=%d, ELEMENTS=%d, ZONETYPE=FETRIANGLE\n", 0.0, Total_number_of_points, Total_number_of_edges);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	
	for (k=0; k<Total_number_of_points; k++)
	{
		N_smb = sprintf( buffer_name,"%f\t%f\t%d\n", Domain[k].getX(), Domain[k].getY(), Domain[k].getType());
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);				
	}
								
	N_smb = sprintf( buffer_name,"\n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
				
	for (m=0; m<Total_number_of_edges; m++)
	{
		// !!! Отличие в экспорте содержится только здесь. Вместо экспорта vector<TriangleCell> DomainCell
		// осуществляем экспорт vector<Edge> DomainEdge
		N_smb = sprintf( buffer_name,"%d %d %d\n", DomainEdge[m].getElement(BEGIN_NODE)+1, DomainEdge[m].getElement(END_NODE)+1, DomainEdge[m].getElement(END_NODE)+1);
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	}

	fclose(f_id_tecplot);
}

//**************************************************************************
// Name: ComputeMinMaxLedge()
// Description: Ищет и запоминает минимальную и максимальную длину ребер
//**************************************************************************

void TrianglePhysicalGeometryData::ComputeMinMaxLedge()
{
	int n;
	double L;
	
	for (n=0; n<Total_number_of_edges; n++)
	{
		Index_M0 = DomainEdge[n].getElement(BEGIN_NODE);
        Index_M2 = DomainEdge[n].getElement(END_NODE);

		if (Domain[Index_M0].getType() == COMPUTATION_AREA || Domain[Index_M2].getType() == COMPUTATION_AREA)
		{					
			// х координата точек
			x_M0 = Domain[Index_M0].getX();
			x_M2 = Domain[Index_M2].getX();
			
			// y координата точек
			y_M0 = Domain[Index_M0].getY();
			y_M2 = Domain[Index_M2].getY();
			
			L = sqrt( (x_M2-x_M0)*(x_M2-x_M0) + (y_M2-y_M0)*(y_M2-y_M0) );
			
			if (n==0)
			{
				L_min = L;
				L_max = L;
			}
			else
			{
				if (L>L_max) L_max = L;
				if (L<L_min) L_min = L;
			}

		}
		// END if (Domain[Index_M0].getType() == COMPUTATION_AREA || Domain[Index_M2].getType() == COMPUTATION_AREA)
	}
	
}

//**************************************************************************
// Name: InitialiseEmptyPhysicalData()
// Description: Выделяет память для хранения физических переменных и присваивает им нулевые значения.
//**************************************************************************

void TrianglePhysicalGeometryData::InitialiseEmptyPhysicalData()
{
	// Выделяем память для перменных
	H.reserve(Total_number_of_points);
	xU.reserve(Total_number_of_points);
	yU.reserve(Total_number_of_points);
	B.reserve(Total_number_of_points);
	
	tau.reserve(Total_number_of_points);
	epsilon.reserve(Total_number_of_points);
	
	ForceX.reserve(Total_number_of_points);
	ForceY.reserve(Total_number_of_points);
	
	Set_for_H_flow.reserve(Total_number_of_points);
	Set_for_H_ux_flow.reserve(Total_number_of_points);
	Set_for_H_uy_flow.reserve(Total_number_of_points);
	
	Set_for_B_dx.reserve(Total_number_of_points);
	Set_for_B_dy.reserve(Total_number_of_points);
	Set_for_H_ux_uy.reserve(Total_number_of_points);
	
	// Присваиваем нулевые значения всем элементам массива
	H.assign(Total_number_of_points, 0.0);
	xU.assign(Total_number_of_points, 0.0);
	yU.assign(Total_number_of_points, 0.0);
	B.assign(Total_number_of_points, 0.0);
	
	tau.assign(Total_number_of_points, 0.0);
	epsilon.assign(Total_number_of_points, 0.0);
	
	ForceX.assign(Total_number_of_points, 0.0);
	ForceY.assign(Total_number_of_points, 0.0);	
			
	Set_for_H_flow.assign(Total_number_of_points, 0.0);
	Set_for_H_ux_flow.assign(Total_number_of_points, 0.0);
	Set_for_H_uy_flow.assign(Total_number_of_points, 0.0);
	
	Set_for_B_dx.assign(Total_number_of_points, 0.0);
	Set_for_B_dy.assign(Total_number_of_points, 0.0);
	Set_for_H_ux_uy.assign(Total_number_of_points, 0.0);
}

//**************************************************************************
// InitialisePhysicalData() -> присвоение значений внутренним массивам
//**************************************************************************

void TrianglePhysicalGeometryData::InitialisePhysicalData() {
		
	//////////////////////////////////////////////////////
	
	// Внутренние переменные функции InitialisePhysicalData():
	int n, i;
	double TEMP;
			
	for (n=0; n<Total_number_of_points; n++)
	{
		x_M0 = Domain[n].getX(); 
		
		if ( x_M0 <= 0.0)
		{
			H[n] = 10.0;
		}
		else
		{
			H[n] = 1.0;
		}		

	}


}

//**************************************************************************
// Name: Initialise_Round_Hump()
// Description: Задает начальное возмущенние в виде круглой колонны
//**************************************************************************

void TrianglePhysicalGeometryData::Initialise_Round_Hump()
{
	//////////////////////////////////////////////////////
	int n, i;
	double R;
		
	for (n=0; n<Total_number_of_points; n++)
	{
		x_M0 = Domain[n].getX(); 
		y_M0 = Domain[n].getY();
		
		R = sqrt( x_M0*x_M0 + y_M0*y_M0 );
		
		if ( R <= 0.05)
		{
			H[n] = 5.0;
		}
		else
		{
			H[n] = 1.0;
		}		

	}	
	


}

//**************************************************************************
// Name: ConstructEpsilon()
// Description: Присвоит значения epsilon по принципу максимального возвышения поверхности. Т.е. epsilon будет равняться разнице высот поверхности дна между данной точкой и максимальной высотой в пределах шаблона.
//**************************************************************************

void TrianglePhysicalGeometryData::ConstructEpsilon()
{
	int n;
	double dB;
	
	// Всем epsilon присваиваем ненулевые значения
	epsilon.assign(Total_number_of_points, 0.1);
	
	for (n=0; n<Total_number_of_edges; n++)
	{
		Index_M0 = DomainEdge[n].getElement(BEGIN_NODE);
		Index_M2 = DomainEdge[n].getElement(END_NODE);
		
		dB = B[Index_M2] - B[Index_M0];

		if (dB > epsilon[Index_M0]) epsilon[Index_M0] = dB;
		
		if (-dB > epsilon[Index_M2]) epsilon[Index_M2] = -dB;		
		
	}

}

//**************************************************************************
// Name: setEpsilonFactor(double eps_factor)
// Description: В случае, если epsilon окажется слишком маленьким, то умножим его на заданный коэффициент
//**************************************************************************

void TrianglePhysicalGeometryData::setEpsilonFactor(double eps_factor)
{
	int k;
	
	for (k=0; k<Total_number_of_points ;k++)
	{
		epsilon[k] = eps_factor*epsilon[k];
	}
	
}

//**************************************************************************
// Name: Initialise_Three_Conical_Humps()
// Description: Метод класса для задания начальных условий в случае трех конических, выпирающих холмиков
//**************************************************************************

void TrianglePhysicalGeometryData::Initialise_Three_Conical_Humps()
{
	int n;
	double x1, y1, x2, y2, x3, y3;
	double rc1, rc2;
	double h1, h2;
	double R1, R2, R3;
	
	x1 = 47.5;
	y1 = 15;
	
	x2 = 30;
	y2 = 5.25;
	
	x3 = 30;
	y3 = 24.75;
	
	rc1 = 10;
	rc2 = 7.5;
	
	h1 = 3;
	h2 = 1;
	
	
	// Сначала задаем B[]
	for (n=0; n<Total_number_of_points; n++)
	{
		x_M0 = Domain[n].getX(); 
		y_M0 = Domain[n].getY();
		
		//Сначала вычислим радиусы для трех точек
		R1 = sqrt( (x_M0-x1)*(x_M0-x1) + (y_M0-y1)*(y_M0-y1) );
		R2 = sqrt( (x_M0-x2)*(x_M0-x2) + (y_M0-y2)*(y_M0-y2) );
		R3 = sqrt( (x_M0-x3)*(x_M0-x3) + (y_M0-y3)*(y_M0-y3) );
		
		if ( R1 <= rc1) B[n] = h1*(1.0 - R1/rc1);
		
		if ( R2 <= rc2) B[n] = h2*(1.0 - R2/rc2);
		
		if ( R3 <= rc2) B[n] = h2*(1.0 - R3/rc2);		

	}	
	
	// Теперь присваиваем значения H
	for (n=0; n<Total_number_of_points; n++)
	{
		x_M0 = Domain[n].getX();
		
		if ( x_M0<16 )
		{
			H[n] = 1.875;
		}
	}
}

//**************************************************************************
// Name: InitialiseTAU()
// Description: Из заданных H, epsilon находит tau
//**************************************************************************

void TrianglePhysicalGeometryData::InitialiseTAU()
{
	int i;

	//**************************************************************************
	//	Заполняем массив TAU
	//**************************************************************************

	for (i=0; i<Total_number_of_points; i++)
	{
		if ( H[i]>epsilon[i] )
		{
			tau[i] = (alpha*(Domain[i].getPerimeter()/Domain[i].getNeighbour()))/sqrt(g*H[i]);
		}
		else
		{
			tau[i] = 0.0;
		}
	}
}

//**************************************************************************
// Name: ExportBottomProfileToTecplot()
// Description: Экспортирует профиль дна. Кроме того вместо переменной B используется переменная Ksi. Чтобы можно было одновременно нарисовать две поверхности
//**************************************************************************

void TrianglePhysicalGeometryData::ExportBottomProfileToTecplot(const char* File_Name)
{

	FILE* f_id_tecplot;
	char buffer_name[500];
	int N_smb;
	
	int k, m;	// Эти переменные нужны для циклов
			
	f_id_tecplot = fopen(File_Name, "w");

	//VARIABLES = X,Y,Ksi
	N_smb = sprintf( buffer_name,"VARIABLES = X,Y,Ksi\n");	// Не забуть изменить "%f\t...\n"
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);

	// ZONE T="Time 0.0", DATAPACKING=POINT, NODES=%d, ELEMENTS=%d, ZONETYPE=FETRIANGLE\n
	N_smb = sprintf( buffer_name,"ZONE T=\"Time %f\", DATAPACKING=POINT, NODES=%d, ELEMENTS=%d, ZONETYPE=FETRIANGLE\n", 0.0, Total_number_of_points, Total_number_of_cells);
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	
	for (k=0; k<Total_number_of_points; k++)
	{
		N_smb = sprintf( buffer_name,"%f\t%f\t%f\n", Domain[k].getX(), Domain[k].getY(), B[k]);
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);		
	}
								
	N_smb = sprintf( buffer_name,"\n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
				
	for (m=0; m<Total_number_of_cells; m++)
	{					
		N_smb = sprintf( buffer_name,"%d %d %d\n", DomainCell[m].getNode(FIRST_NODE)+1, DomainCell[m].getNode(SECOND_NODE)+1, DomainCell[m].getNode(THIRD_NODE)+1);
		fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	}
	
	N_smb = sprintf( buffer_name,"\n");
	fwrite(&buffer_name, sizeof(char), N_smb, f_id_tecplot);
	
	fclose(f_id_tecplot);

}

//========================================
