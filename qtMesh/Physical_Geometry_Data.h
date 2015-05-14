// Physical_Geometry_Data.h
// Дата последней модификации: 29 января 2014

// Проблемы для ликвидации:
// Когда задаешь шаблон класса нельзя присваивать значения см.
// double S_control = hx*hy;

#include <vector>
using namespace std;

#include "Geometry_Mesh.h"

#ifndef PHYSICAL_GEOMETRY_DATA_H
#define PHYSICAL_GEOMETRY_DATA_H

#define CONTINUE_SHALLOW_WATER_COMPUTATION	14
#define ERROR_SHALLOW_WATER_COMPUTATION		58

class SquarePhysicalGeometryData {

public:
	SquarePhysicalGeometryData();
	~SquarePhysicalGeometryData();
		
	void ExportDataToTecplot(const char* file, double Time);	// Нельзя поменять то на что указывает ссылка, но можно поменять содержание того, на что указывает указатель. Т.е. грубо говоря можно перейти по стрелке/ссылке и менять. Но нельзя изменять направление стрелки/ссылки.
	
	void AddDataToTecplot(const char* file, double Time); // Добавляет данные в уже существующий Текплот-файл

	void size();
	

	void setDomain(int Nx, int Ny, const double* X, const double* Y, const int* S);	// Присвивает значения vector<Point> Domain
	
	void InitialiseGeometry(int Nx, int Ny, const int* S); // Заполняем vector<SquareCell> DomainCell; vector<Edge> DomainEdge
	
	void ExportGeometry(const char* file_name); // Записывает геометрические данные в текстовый файл. Этот метод нужен только для отладки правильной работы программы.
	
	void InitialisePhysicalData();	// Сюда поместим алгоритм, который присвоит значения 
	
	void ComputeAverageLength(); // Отдельный алгоритм для поиска характерной длины расчетной ячейки
	
	double getAverageLength(); // Возвращает средний размер расчетной ячейки
	
	double maxH(); // Находит и возвращает максимальное значение H
	
	int ComputeNextTimeLayer(double dT); // При заданном dT находим значения на следующем слое
	
	void setGravity(double temp_g=9.81) { g = temp_g; }
	
	double getGravity() { return g; }
	
	void setAlpha(double temp_alpha) { alpha = temp_alpha; }
	
	double getAlpha() { return alpha; }
	
	void setBeta(double temp_beta) { beta = temp_beta; }
	
	double getBeta() {return beta;}
	
	void ImportFemlabPoints(const char* file_name); // Импортирует из бинарного файла информацию о координатах точек
	
	void ImportFemlabTriangles(const char* file_name);	// Импортирует из бинарного файла информацию о треугольниках сетки
	
	void Temp_InitialiseTypeOfPoints();	// Временная функция. Присваивает типы точкам в ручном режиме
	
	void ExportGeometryToTecplot(const char* file); // Записывает в файл только сетку
	
private:
	int Total_number_of_points;	// Общее число точек
	int Total_number_of_cells;	// Общее число элементов (треугольников или прямоугольников)
	int Total_number_of_edges;	// Общее число ребер
	
	//************************************************
	//	Основные массивы для геометрии области
	//************************************************
	
	vector<Point> Domain;
	
	Point TempPoint; // Временный объект, который служит для временного хранения данных
	
	
	vector<SquareCell> DomainCell;
	
	SquareCell TempElement; // Временный объект

	vector<Edge> DomainEdge;
	
	Edge TempEdge;	// Временный объект
	
	//************************************************
	//	Служебные перменные, которые нужны для численного расчета
	//************************************************
	
	double g;
	double alpha; 
	double beta;
	
	int edge_1, edge_2;
	int cell_1, cell_2, cell_3, cell_4;
	int Type_p1, Type_p2;

	double L_average; // средняя длина ребра, используется для вычисленя Dt, tau
	double MAX_H;
	
	double S_control;

	int Index_P1, Index_P2, Index_M0, Index_M2;

	int Index_p1_T1, Index_p2_T1, Index_p3_T1, Index_p4_T1;
	int Index_p1_T2, Index_p2_T2, Index_p3_T2, Index_p4_T2;

	double x_M0, x_M2, x_P1, x_P2; 
	double	y_M0, y_M2, y_P1, y_P2;

	double H_M0, H_M2, H_P1, H_P2, H_P32;
	double xU_M0, xU_M2, xU_P1, xU_P2, xU_P32;
	double yU_M0, yU_M2, yU_P1, yU_P2, yU_P32;
	double B_M0, B_M2, B_P1, B_P2, B_P32;
	
	double ForceX_P1, ForceX_P2, ForceX_P32;
	double ForceY_P1, ForceY_P2, ForceY_P32;
	
	double tau_P32;
	double S_edge, L_P1P2;
	double nx_P32, ny_P32;

	////////////////////////////////////////////////////////////////////

	double xJ_P32, yJ_P32;
	double xWs_P32, yWs_P32, Rs_P32;
	double xxPT_P32, xyPT_P32, yxPT_P32, yyPT_P32;
	
	//************************************************
	//	Массивы для хранения физических данных
	//************************************************
	
	vector<double> H;
	vector<double> xU;
	vector<double> yU;
	vector<double> B;
	
	vector<double> tau;
	vector<double> epsilon;
	
	vector<double> ForceX;
	vector<double> ForceY;
	
	vector<double> Set_for_H_flow;
	vector<double> Set_for_H_ux_flow;
	vector<double> Set_for_H_uy_flow;
	
	vector<double> Set_for_B_dx;
	vector<double> Set_for_B_dy;
	vector<double> Set_for_H_ux_uy;

};

class TrianglePhysicalGeometryData {

public:
	TrianglePhysicalGeometryData();
	~TrianglePhysicalGeometryData();
		
	void ExportDataToTecplot(const char* file, double Time);	// Нельзя поменять то на что указывает ссылка, но можно поменять содержание того, на что указывает указатель. Т.е. грубо говоря можно перейти по стрелке/ссылке и менять. Но нельзя изменять направление стрелки/ссылки.
	
	void AddDataToTecplot(const char* file, double Time); // Добавляет данные в уже существующий Текплот-файл

	void size();
	

	void setDomain(int Nx, int Ny, const double* X, const double* Y, const int* S);	// Присвивает значения vector<Point> Domain
	
	void ConstructEdges(); // Заполняем vector<SquareCell> DomainCell; vector<Edge> DomainEdge
	
	void ExportGeometry(const char* file_name); // Записывает геометрические данные в текстовый файл. Этот метод нужен только для отладки правильной работы программы.
	
	
	void ComputeAverageLength(); // Отдельный алгоритм для поиска характерной длины расчетной ячейки
	
	double getAverageLength(); // Возвращает средний размер расчетной ячейки
	
	double maxH(); // Находит и возвращает максимальное значение H
	
	int ComputeNextTimeLayer(double dT); // При заданном dT находим значения на следующем слое
	
	void setGravity(double temp_g=9.81) { g = temp_g; }
	
	double getGravity() { return g; }
	
	void setAlpha(double temp_alpha) { alpha = temp_alpha; }
	
	double getAlpha() { return alpha; }
	
	void setBeta(double temp_beta) { beta = temp_beta; }
	
	double getBeta() {return beta;}
	
	void ImportFemlabPoints(const char* file_name); // Импортирует из бинарного файла информацию о координатах точек
	
	void ImportFemlabTriangles(const char* file_name);	// Импортирует из бинарного файла информацию о треугольниках сетки
	
	void ConfirmCounterClockwise(); // Меняет номера точек так, чтобы номера точек треугольник шли против часовой стрелки
	
	void Temp_InitialiseTypeOfPoints(double Xmin, double Xmax, double Ymin, double Ymax);	// Временная функция. Присваивает типы точкам в ручном режиме
	
	void InitialisePhysicalData();	// Сюда поместим алгоритм, который присвоит значения 
		
	void ExportGeometryToTecplot(const char* file_name); // Записывает в файл только сетку
	
	void ExportEdgesToTecplot(const char* file_name);  // Записывает в файл Tecplot только ребра
	
	void ComputeMinMaxLedge();	// Ищет и запоминает минимальную и максимальную длину ребер
	
	void Initialise_Round_Hump(); // Задает начальное возмущенние в виде круглой колонны
	
	void ConstructEpsilon(); // Присвоит значения epsilon по принципу максимального возвышения поверхности. Т.е. epsilon будет равняться разнице высот поверхности дна между данной точкой и максимальной высотой в пределах шаблона.

	void setEpsilonFactor(double eps_factor);	// В случае, если epsilon окажется слишком маленьким, то умножим его на заданный коэффициент
	
	void Initialise_Three_Conical_Humps(); // Метод класса для задания начальных условий в случае трех конических, выпирающих холмиков
	
	void InitialiseEmptyPhysicalData();	// Выделяет память для хранения физических переменных и присваивает им нулевые значения.
	
	void InitialiseTAU();	// Из заданных H, epsilon находит tau
	
	void ExportBottomProfileToTecplot(const char* File_Name); // Экспортирует профиль дна. Кроме того вместо переменной B используется переменная Ksi. Чтобы можно было одновременно нарисовать две поверхности
    
    void exportDataToTxtFile(); // Запишем прочитанную информацию в текстовый файл
	
private:
	int Total_number_of_points;	// Общее число точек
	int Total_number_of_cells;	// Общее число элементов (треугольников или прямоугольников)
	int Total_number_of_edges;	// Общее число ребер
	
	//************************************************
	//	Основные массивы для геометрии области
	//************************************************
	
	vector<Point> Domain;
	
	Point TempPoint; // Временный объект, который служит для временного хранения данных
	
	
	vector<TriangleCell> DomainCell;
	
	TriangleCell TempElement; // Временный объект

	vector<Edge> DomainEdge;
	
	Edge TempEdge;	// Временный объект
	
	//************************************************
	//	Служебные перменные, которые нужны для численного расчета
	//************************************************
	
	double g;
	double alpha; 
	double beta;
	
	int edge_1, edge_2;
	int cell_1, cell_2, cell_3;
	int Type_p1, Type_p2;

	double L_average; // средняя длина ребра, используется для вычисленя Dt, tau
	double MAX_H;
	double L_min, L_max;
	
	double S_control;

	int Index_P1, Index_P2, Index_M0, Index_M2;

	// !?!? Нужно удалить потом лишние элементы (вместе с cpp-файлом) Index_p4_T1, Index_p4_T2
	int Index_p1_T1, Index_p2_T1, Index_p3_T1;
	int Index_p1_T2, Index_p2_T2, Index_p3_T2;

	double x_M0, x_M2, x_P1, x_P2; 
	double	y_M0, y_M2, y_P1, y_P2;

	double H_M0, H_M2, H_P1, H_P2, H_P32;
	double xU_M0, xU_M2, xU_P1, xU_P2, xU_P32;
	double yU_M0, yU_M2, yU_P1, yU_P2, yU_P32;
	double B_M0, B_M2, B_P1, B_P2, B_P32;
	
	double ForceX_P1, ForceX_P2, ForceX_P32;
	double ForceY_P1, ForceY_P2, ForceY_P32;
	
	double tau_P32;
	double S_edge, L_P1P2;
	double nx_P32, ny_P32;

	////////////////////////////////////////////////////////////////////

	double xJ_P32, yJ_P32;
	double xWs_P32, yWs_P32, Rs_P32;
	double xxPT_P32, xyPT_P32, yxPT_P32, yyPT_P32;
	
	//************************************************
	//	Массивы для хранения физических данных
	//************************************************
	
	vector<double> H;
	vector<double> xU;
	vector<double> yU;
	vector<double> B;
	
	vector<double> tau;
	vector<double> epsilon;
	
	vector<double> ForceX;
	vector<double> ForceY;
	
	vector<double> Set_for_H_flow;
	vector<double> Set_for_H_ux_flow;
	vector<double> Set_for_H_uy_flow;
	
	vector<double> Set_for_B_dx;
	vector<double> Set_for_B_dy;
	vector<double> Set_for_H_ux_uy;

};

#endif