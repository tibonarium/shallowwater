// Geometry_Mesh.h
// Дата последней модификации: 31 декабря 2013

#ifndef GEOMETRY_MESH_H
#define GEOMETRY_MESH_H

enum TypeNode { FIRST_NODE, SECOND_NODE, THIRD_NODE, FOURTH_NODE };
enum TypeElement { BEGIN_NODE, END_NODE, LEFT_ELEMENT, RIGHT_ELEMENT };

//**************************************************************************
// Class name: Point
// Description: Класс для хранения информации о типе и координатах точки
//**************************************************************************

class Point {

	double x;
	double y;
	int type;
	int neighbour_count;
	double perimeter;
	double control_volume;

public:

	double getX() { return x; }

	void setX(double temp_x) { x = temp_x; }

	double getY() { return y; }

	void setY(double temp_y) { y = temp_y; }

	int getType() {return type;}

	void setType(int temp_type) { type = temp_type; }

	int getNeighbour() {return neighbour_count;}

	void setNeighbour(int temp_neighbour_count=0) { neighbour_count = temp_neighbour_count; }
	
	void setValues(double temp_x, double temp_y, int temp_type, int temp_neighbour_count=0) {
		x = temp_x;
		y = temp_y;
		type = temp_type;
		neighbour_count = temp_neighbour_count;
		perimeter = 0.0;
		control_volume = 0.0;
	}
	
	void addPerimeter(double temp_length) { perimeter += temp_length; }
	
	double getPerimeter() { return perimeter; }
	
	void addControlVolume(double temp_control_volume) { control_volume += temp_control_volume; }
	
	double getControlVolume() { return control_volume; }

};

//**************************************************************************
// Class name: SquareCell
// Description: Класс для хранения информации об элементе в виде прямоугольника
//**************************************************************************

class SquareCell {

	int first_node;
	int second_node;
	int third_node;
	int fourth_node;
public:
	void setFirstNode(int i1) { first_node = i1; }
	void setSecondNode(int i2) { second_node = i2; }
	void setThirdNode(int i3) { third_node = i3; }
	void setFourthNode(int i4) { fourth_node = i4; }
	
	void setElement(int i1, int i2, int i3, int i4) {
		first_node = i1;
		second_node = i2;
		third_node = i3;
		fourth_node = i4;
	}
	
	int getNode(TypeNode type_node) {
	
		switch(type_node)
		{
			case FIRST_NODE:
			return first_node;
			break;
			
			case SECOND_NODE:
			return second_node;
			break;
			
			case THIRD_NODE:
			return third_node;
			break;
			
			case FOURTH_NODE:
			return fourth_node;
			break;
		
		}
	
	
	}
	
};

//**************************************************************************
// Class name: TriangleCell
// Description: Класс для хранения информации об элементе в виде треугольника
//**************************************************************************

class TriangleCell {

	int first_node;
	int second_node;
	int third_node;

public:
	void setFirstNode(int i1) { first_node = i1; }
	void setSecondNode(int i2) { second_node = i2; }
	void setThirdNode(int i3) { third_node = i3; }
	
	void setElement(int i1, int i2, int i3)
	{
		first_node = i1;
		second_node = i2;
		third_node = i3;
	}
	
	int getNode(TypeNode type_node)
	{	
		switch(type_node)
		{
			case FIRST_NODE:
			return first_node;
			break;
			
			case SECOND_NODE:
			return second_node;
			break;
			
			case THIRD_NODE:
			return third_node;
			break;
		}
	
	}
    	
};

//**************************************************************************
// Class name: Edge
// Description: Класс для хранения информации о ребре
//**************************************************************************

class Edge {

	int begin_node;
	int end_node;
	int left_element;
	int right_element;
public:
	void setBeginNode(int i1) { begin_node = i1; }
	void setEndNode(int i2) { end_node = i2; }
	void setLeftElement(int e1) { left_element = e1; }
	void setRightElement(int e2) { right_element = e2; }
	
	void setEdge(int i1, int i2, int e1=-1, int e2=-1) {
		begin_node = i1;
		end_node = i2;
		left_element = e1;
		right_element = e2;	
	}
	
	int getElement(TypeElement type_element) {
		
		switch(type_element)
		{
			case BEGIN_NODE:
			return begin_node;
			break;
			
			case END_NODE:
			return end_node;
			break;
			
			case LEFT_ELEMENT:
			return left_element;
			break;
			
			case RIGHT_ELEMENT:
			return right_element;
			break;
		}
	
	}

};

#endif