#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	vector3 pointTop(0, 0, a_fHeight / 2); // Top of cone
	vector3 pointBottom(0, 0, -(a_fHeight / 2)); // Bottom of cone

	// Calculate the angle of difference (in radians) between all subdivision points
	float proportion = (((2.0 * PI) * (360.0 / (float)a_nSubdivisions)) / 360.0);

	// Calculate the first point for the circle and store it
	vector3 lastPoint = vector3(a_fRadius * (cos(proportion)), a_fRadius * (sin(proportion)), -(a_fHeight / 2));
	vector3 currentPoint;

	// Create all the triangles for the circle
	for (int i = 2; i < a_nSubdivisions + 1; i++)
	{
		// Calculate the current point of the subdivision
		currentPoint = vector3(a_fRadius * (cos(proportion * i)), a_fRadius * (sin(proportion * i)), -(a_fHeight / 2));

		// Add triangle where
		//	a is the previously calculated point,
		//	b is the current calculated point,
		//	c is the center of the base
		AddTri(lastPoint, currentPoint, pointBottom);

		// Add another triangle where
		//	a is the previously calculated point,
		//	b is the current calculated point,
		//	c is the top of the cone
		AddTri(currentPoint, lastPoint, pointTop);

		// Set this calculated point to the last point for the next triangles
		lastPoint = currentPoint;
	}

	// Finish with the last triangle
	AddTri(lastPoint, vector3(a_fRadius * (cos(proportion)), a_fRadius * (sin(proportion)), -(a_fHeight / 2)), pointBottom);
	AddTri(vector3(a_fRadius * (cos(proportion)), a_fRadius * (sin(proportion)), -(a_fHeight / 2)), lastPoint, pointTop);

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	vector3 top(0, 0, a_fHeight); // Top center of cylinder
	vector3 bottom(0, 0, -(a_fHeight / 2)); // Bottom center of cylinder

	// Calculate the angle of difference (in radians) between all subdivision points
	float proportion = (((2.0 * PI) * (360.0 / (float)a_nSubdivisions)) / 360.0);

	// Calculate the first point for the circle and store it
	vector3 lastPoint = vector3(a_fRadius * (cos(proportion)), a_fRadius * (sin(proportion)), -(a_fHeight / 2));
	vector3 currentPoint;

	// Create all the triangles for the circle
	for (int i = 2; i < a_nSubdivisions + 1; i++)
	{
		// Calculate the current point of the subdivision
		currentPoint = vector3(a_fRadius * (cos(proportion * i)), a_fRadius * (sin(proportion * i)), -(a_fHeight / 2));

		// Add triangle where
		//	a is the current calculated point,
		//	b is the previously calculated point,
		//	c is the center of the base
		AddTri(currentPoint, lastPoint, bottom);

		// Add another triangle where
		//	a is the previously calculated point at the top of the cylinder,
		//	b is the current calculated point at the top of the cylinder,
		//	c is the center of the top
		AddTri(lastPoint + top, currentPoint + top, bottom + top);

		// Add a quad where
		//	a is the previously calculated point,
		//	b is the current calculated point,
		//	c is the current calculated point at the top of the cylinder,
		//	d is the current previously point at the top of the cylinder
		AddQuad(lastPoint, currentPoint, lastPoint + top, currentPoint + top);

		// Set this calculated point to the last point for the next triangles and quad
		lastPoint = currentPoint;
	}

	// Finish with the last triangles and quad
	AddTri(vector3(a_fRadius * (cos(proportion)), a_fRadius * (sin(proportion)), -(a_fHeight / 2)), lastPoint, bottom);
	AddTri(lastPoint + top, vector3(a_fRadius * (cos(proportion)), a_fRadius * (sin(proportion)), -(a_fHeight / 2)) + top, bottom + top);
	AddQuad(lastPoint, vector3(a_fRadius * (cos(proportion)), a_fRadius * (sin(proportion)), -(a_fHeight / 2)), lastPoint + top, vector3(a_fRadius * (cos(proportion)), a_fRadius * (sin(proportion)), -(a_fHeight / 2)) + top);

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	vector3 top(0, 0, a_fHeight); // Top center of tube

	// Calculate the angle of difference (in radians) between all subdivision points
	float proportion = (((2.0 * PI) * (360.0 / (float)a_nSubdivisions)) / 360.0);

	// Calculate the first outer and inner points for the circle and store it
	vector3 lastOuterPoint = vector3(a_fOuterRadius * (cos(proportion)), a_fOuterRadius * (sin(proportion)), -(a_fHeight / 2));
	vector3 lastInnerPoint = vector3(a_fInnerRadius * (cos(proportion)), a_fInnerRadius * (sin(proportion)), -(a_fHeight / 2));
	vector3 currentOuterPoint;
	vector3 currentInnerPoint;

	// Create all the triangles for the circle
	for (int i = 2; i < a_nSubdivisions + 1; i++)
	{
		// Calculate the current outer and inner point of the subdivision
		currentOuterPoint = vector3(a_fOuterRadius * (cos(proportion * i)), a_fOuterRadius * (sin(proportion * i)), -(a_fHeight / 2));
		currentInnerPoint = vector3(a_fInnerRadius * (cos(proportion * i)), a_fInnerRadius * (sin(proportion * i)), -(a_fHeight / 2));

		// Add quad where
		//	a is the previously calculated outer point,
		//	b is the current calculated outer point,
		//	c is the previously calculated outer point at the top of the tube,
		//	d is the current calculated outer point at the top of the tube
		AddQuad(lastOuterPoint, currentOuterPoint, lastOuterPoint + top, currentOuterPoint + top);

		// Add quad where
		//	a is the current calculated inner point,
		//	b is the previously calculated inner point,
		//	c is the current calculated inner point at the top of the tube,
		//	d is the previously calculated inner point at the top of the tube
		AddQuad(currentInnerPoint, lastInnerPoint, currentInnerPoint + top, lastInnerPoint + top);

		// Add quad where
		//	a is the previously calculated inner point,
		//	b is the current calculated inner point,
		//	c is the previously calculated outer point,
		//	d is the current calculated outer point,
		AddQuad(lastInnerPoint, currentInnerPoint, lastOuterPoint, currentOuterPoint);

		// Add quad where
		//	a is the current calculated inner point at the top of the tube,
		//	b is the previously calculated inner point at the top of the tube,
		//	c is the current calculated outer point at the top of the tube,
		//	d is the previously calculated outer point at the top of the tube,
		AddQuad(currentInnerPoint + top, lastInnerPoint + top, currentOuterPoint + top, lastOuterPoint + top);

		// Set these calculated points to the last point for the next quads
		lastInnerPoint = currentInnerPoint;
		lastOuterPoint = currentOuterPoint;
	}

	// Calculate final points
	vector3 finalOuterPoint(a_fOuterRadius * (cos(proportion)), a_fOuterRadius * (sin(proportion)), -(a_fHeight / 2));
	vector3 finalInnerPoint(a_fInnerRadius * (cos(proportion)), a_fInnerRadius * (sin(proportion)), -(a_fHeight / 2));

	// Finish with the last quads
	AddQuad(lastOuterPoint, finalOuterPoint, lastOuterPoint + top, finalOuterPoint + top);
	AddQuad(finalInnerPoint, lastInnerPoint, finalInnerPoint + top, lastInnerPoint + top);
	AddQuad(lastInnerPoint, finalInnerPoint, lastOuterPoint, finalOuterPoint);
	AddQuad(finalInnerPoint + top, lastInnerPoint + top, finalOuterPoint + top, lastOuterPoint + top);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	float circleRadius = (a_fOuterRadius - a_fInnerRadius) / 2; // Radius of the circles that make up the torus

	// Calculate the angle of difference (in radians) between all subdivision points
	float proportionA = (((2.0 * PI) * (360.0 / (float)a_nSubdivisionsA)) / 360.0);
	float proportionB = (((2.0 * PI) * (360.0 / (float)a_nSubdivisionsB)) / 360.0);

	// Calculate the edge points of the first circle
	vector3 circleEdgeA = vector3(a_fInnerRadius * (cos(proportionA)), a_fInnerRadius * (sin(proportionA)), 0);
	vector3 circleEdgeB = vector3(a_fOuterRadius * (cos(proportionA)), a_fOuterRadius * (sin(proportionA)), 0);
	vector3 currentRelativePoint;
	vector3 finalRelativePoint;
	vector3 finalCirclePoint;
	vector3 currentCirclePoints[361];
	vector3 lastCirclePoints[361];

	// Calculate the first point on the circle
	currentRelativePoint = vector3(circleRadius * (sin(proportionB)), circleRadius * (sin(proportionB)), circleRadius * (cos(proportionB)));
	lastCirclePoints[0] = vector3((((currentRelativePoint.x + circleRadius) / (circleRadius * 2)) * (circleEdgeB.x - circleEdgeA.x)) + circleEdgeA.x, (((currentRelativePoint.y + circleRadius) / (circleRadius * 2)) * (circleEdgeB.y - circleEdgeA.y)) + circleEdgeA.y, currentRelativePoint.z);

	// Calculate the first circle's points
	for (int i = 2; i <= a_nSubdivisionsB; i++)
	{
		currentRelativePoint = vector3(circleRadius * (sin(proportionB * i)), circleRadius * (sin(proportionB * i)), circleRadius * (cos(proportionB * i)));
		lastCirclePoints[i - 1] = vector3((((currentRelativePoint.x + circleRadius) / (circleRadius * 2)) * (circleEdgeB.x - circleEdgeA.x)) + circleEdgeA.x, (((currentRelativePoint.y + circleRadius) / (circleRadius * 2)) * (circleEdgeB.y - circleEdgeA.y)) + circleEdgeA.y, currentRelativePoint.z);
	}

	// Calculate all remaining circles on the torus
	for (int i = 2; i <= a_nSubdivisionsA; i++)
	{
		// Calculate the edge points of the circle
		vector3 circleEdgeA = vector3(a_fInnerRadius * (cos(proportionA * i)), a_fInnerRadius * (sin(proportionA * i)), 0);
		vector3 circleEdgeB = vector3(a_fOuterRadius * (cos(proportionA * i)), a_fOuterRadius * (sin(proportionA * i)), 0);

		// Calculate first point on the circle
		currentRelativePoint = vector3(circleRadius * (sin(proportionB)), circleRadius * (sin(proportionB)), circleRadius * (cos(proportionB)));
		currentCirclePoints[0] = vector3((((currentRelativePoint.x + circleRadius) / (circleRadius * 2)) * (circleEdgeB.x - circleEdgeA.x)) + circleEdgeA.x, (((currentRelativePoint.y + circleRadius) / (circleRadius * 2)) * (circleEdgeB.y - circleEdgeA.y)) + circleEdgeA.y, currentRelativePoint.z);

		// Calculate the points on the circle
		for (int j = 2; j <= a_nSubdivisionsB; j++)
		{
			// Calculate new point on circle
			currentRelativePoint = vector3(circleRadius * (sin(proportionB * j)), circleRadius * (sin(proportionB * j)), circleRadius * (cos(proportionB * j)));
			currentCirclePoints[j - 1] = vector3((((currentRelativePoint.x + circleRadius) / (circleRadius * 2)) * (circleEdgeB.x - circleEdgeA.x)) + circleEdgeA.x, (((currentRelativePoint.y + circleRadius) / (circleRadius * 2)) * (circleEdgeB.y - circleEdgeA.y)) + circleEdgeA.y, currentRelativePoint.z);

			// Add quad where
			//	a is the current calculated point
			//	b is the previously calculated point
			//	c is the current indexed point on the previous layer
			//	d is the previously indexed point on the previous layer
			AddQuad(currentCirclePoints[j - 1], currentCirclePoints[j - 2], lastCirclePoints[j - 1], lastCirclePoints[j - 2]);
		}

		// Calculate the final point on the circle
		finalRelativePoint = vector3(circleRadius * (sin(proportionB)), circleRadius * (sin(proportionB)), circleRadius * (cos(proportionB)));
		finalCirclePoint = vector3((((finalRelativePoint.x + circleRadius) / (circleRadius * 2)) * (circleEdgeB.x - circleEdgeA.x)) + circleEdgeA.x, (((finalRelativePoint.y + circleRadius) / (circleRadius * 2)) * (circleEdgeB.y - circleEdgeA.y)) + circleEdgeA.y, finalRelativePoint.z);

		// Add final quad
		AddQuad(finalCirclePoint, currentCirclePoints[a_nSubdivisionsB - 1], lastCirclePoints[0], lastCirclePoints[a_nSubdivisionsB - 1]);

		// Set current points to last points
		for (int i = 0; i < a_nSubdivisionsB; i++)
		{
			lastCirclePoints[i] = currentCirclePoints[i];
		}
	}

	// Calculate final points
	circleEdgeA = vector3(a_fInnerRadius * (cos(proportionA)), a_fInnerRadius * (sin(proportionA)), 0);
	circleEdgeB = vector3(a_fOuterRadius * (cos(proportionA)), a_fOuterRadius * (sin(proportionA)), 0);

	// Calculate first point on the circle
	currentRelativePoint = vector3(circleRadius * (sin(proportionB)), circleRadius * (sin(proportionB)), circleRadius * (cos(proportionB)));
	currentCirclePoints[0] = vector3((((currentRelativePoint.x + circleRadius) / (circleRadius * 2)) * (circleEdgeB.x - circleEdgeA.x)) + circleEdgeA.x, (((currentRelativePoint.y + circleRadius) / (circleRadius * 2)) * (circleEdgeB.y - circleEdgeA.y)) + circleEdgeA.y, currentRelativePoint.z);

	// Calculate the first circle's points
	for (int i = 2; i <= a_nSubdivisionsB; i++)
	{
		// Calculate new point on circle
		currentRelativePoint = vector3(circleRadius * (sin(proportionB * i)), circleRadius * (sin(proportionB * i)), circleRadius * (cos(proportionB * i)));
		currentCirclePoints[i - 1] = vector3((((currentRelativePoint.x + circleRadius) / (circleRadius * 2)) * (circleEdgeB.x - circleEdgeA.x)) + circleEdgeA.x, (((currentRelativePoint.y + circleRadius) / (circleRadius * 2)) * (circleEdgeB.y - circleEdgeA.y)) + circleEdgeA.y, currentRelativePoint.z);

		// Add quad where
			//	a is the current calculated point
			//	b is the previously calculated point
			//	c is the current indexed point on the previous layer
			//	d is the previously indexed point on the previous layer
		AddQuad(currentCirclePoints[i - 1], currentCirclePoints[i - 2], lastCirclePoints[i - 1], lastCirclePoints[i - 2]);
	}

	// Calculate the final point on the circle
	finalRelativePoint = vector3(circleRadius * (sin(proportionB)), circleRadius * (sin(proportionB)), circleRadius * (cos(proportionB)));
	finalCirclePoint = vector3((((finalRelativePoint.x + circleRadius) / (circleRadius * 2)) * (circleEdgeB.x - circleEdgeA.x)) + circleEdgeA.x, (((finalRelativePoint.y + circleRadius) / (circleRadius * 2)) * (circleEdgeB.y - circleEdgeA.y)) + circleEdgeA.y, finalRelativePoint.z);

	// Add final quad
	AddQuad(finalCirclePoint, currentCirclePoints[a_nSubdivisionsB - 1], lastCirclePoints[0], lastCirclePoints[a_nSubdivisionsB - 1]);

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 99)
		a_nSubdivisions = 99;

	Release();
	Init();

	// Replace this with your code
	vector3 previousLayerTop[99]; // Last layer of the top half of the sphere
	vector3 currentLayerTop[99]; // Current layer of the top half of the sphere
	vector3 previousLayerBottom[99]; // Last layer of the bottom half of the sphere
	vector3 currentLayerBottom[99]; // Current layer of the bottom half of the sphere
	vector3 top(0, 0, a_fRadius); // Top of the sphere
	vector3 bottom(0, 0, -a_fRadius); // Bottom of the sphere

	// Calculate the angle of difference (in radians) between all subdivision points
	float proportion = (((2.0 * PI) * (360.0 / (float)a_nSubdivisions)) / 360.0);
	float proportionSphere = ((PI / 4) * (90.0 / ((float)(a_nSubdivisions - 1) / 4.0))) / 90.0;;

	// Calculate the first points for the layers of the sphere and store it
	float layerRadius = sin(proportionSphere) * a_fRadius;
	float layerHeight = cos(proportionSphere) * a_fRadius;
	vector3 lastPointTop = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), layerHeight);
	vector3 currentPointTop;
	vector3 lastPointBottom = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), -layerHeight);
	vector3 currentPointBottom;
	previousLayerTop[0] = lastPointTop;
	previousLayerBottom[0] = lastPointBottom;

	// Calculate the first layers of points
	for (int i = 2; i <= a_nSubdivisions; i++)
	{
		// Calculate the current point of the subdivision
		currentPointTop = vector3(layerRadius * (cos(proportion * i)), layerRadius * (sin(proportion * i)), layerHeight);
		currentPointBottom = vector3(layerRadius * (cos(proportion * i)), layerRadius * (sin(proportion * i)), -layerHeight);

		// Add triangle where
		//	a is the previously calculated point on the top,
		//	b is the current calculated point on the top,
		//	c is the top of the sphere
		AddTri(lastPointTop, currentPointTop, top);

		// Add triangle where
		//	a is the current calculated point on the bottom,
		//	b is the previously calculated point on the bottom,
		//	c is the bottom of the sphere
		AddTri(currentPointBottom, lastPointBottom, bottom);

		// Set the calculated points to the last points for the next triangles and layers
		lastPointTop = currentPointTop;
		lastPointBottom = currentPointBottom;
		previousLayerTop[i - 1] = currentPointTop;
		previousLayerBottom[i - 1] = currentPointBottom;
	}

	// Calculate final point
	currentPointTop = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), layerHeight);
	currentPointBottom = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), -layerHeight);

	// Finish layers with final triangles
	AddTri(lastPointTop, currentPointTop, top);
	AddTri(currentPointBottom, lastPointBottom, bottom);

	// Different cases for odd or even subdivisions
	if (a_nSubdivisions % 2 == 0)
	{
		// Calculate the layers of the sphere
		for (int i = 2; i <= a_nSubdivisions / 2; i++)
		{
			// Calculate new layer height and radius
			layerRadius = sin(proportionSphere * i) * a_fRadius;
			layerHeight = cos(proportionSphere * i) * a_fRadius;

			// Calculate first points in new layers
			lastPointTop = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), layerHeight);
			lastPointBottom = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), -layerHeight);
			currentLayerTop[0] = lastPointTop;
			currentLayerBottom[0] = lastPointBottom;

			// Calculate layer of points
			for (int j = 1; j < a_nSubdivisions; j++)
			{
				// Calculate the current points of the subdivision
				currentPointTop = vector3(layerRadius * (cos(proportion * (j + 1))), layerRadius * (sin(proportion * (j + 1))), layerHeight);
				currentPointBottom = vector3(layerRadius * (cos(proportion * (j + 1))), layerRadius * (sin(proportion * (j + 1))), -layerHeight);

				// Add quad where
				//	a is the previously calculated point on the top
				//	b is the current calculated point on the top
				//	c is the previously indexed point on the top on the previous layer
				//	d is the current indexed point on the top on the previous layer
				AddQuad(lastPointTop, currentPointTop, previousLayerTop[j - 1], previousLayerTop[j]);

				// Add quad where
				//	a is the current calculated point on the bottom
				//	b is the previously calculated point on the bottom
				//	c is the current indexed point on the bottom on the previous layer
				//	d is the previously indexed point on the bottom on the previous layer
				AddQuad(currentPointBottom, lastPointBottom, previousLayerBottom[j], previousLayerBottom[j - 1]);

				// Set the calculated points to the last points for the next triangles and layers
				lastPointTop = currentPointTop;
				currentLayerTop[j] = currentPointTop;
				lastPointBottom = currentPointBottom;
				currentLayerBottom[j] = currentPointBottom;
			}

			// Calculate final points
			currentPointTop = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), layerHeight);
			currentPointBottom = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), -layerHeight);

			// Finish layers with final quads
			AddQuad(lastPointTop, currentPointTop, previousLayerTop[a_nSubdivisions - 1], previousLayerTop[0]);
			AddQuad(currentPointBottom, lastPointBottom, previousLayerBottom[0], previousLayerBottom[a_nSubdivisions - 1]);

			// Set current layer to previous layer
			for (int j = 0; j < a_nSubdivisions; j++)
			{
				previousLayerTop[j] = currentLayerTop[j];
				previousLayerBottom[j] = currentLayerBottom[j];
			}
		}

		// Add quad
		AddQuad(previousLayerBottom[0], previousLayerBottom[a_nSubdivisions - 1], previousLayerTop[0], previousLayerTop[a_nSubdivisions - 1]);

		// Connect remaining points
		for (int i = 1; i < a_nSubdivisions; i++)
		{
			// Add quad where
				//	a is the previously calculated point on the bottom
				//	b is the previously indexed point on the bottom on the previous layer
				//	a is the previously calculated point on the top
				//	b is the previously indexed point on the top on the previous layer
			AddQuad(previousLayerBottom[i], previousLayerBottom[i - 1], previousLayerTop[i], previousLayerTop[i - 1]);
		}
	}
	else
	{
		// Calculate the layers of the sphere
		for (int i = 2; i <= (a_nSubdivisions + 1) / 2; i++)
		{
			// Calculate new layer height and radius
			layerRadius = sin(proportionSphere * i) * a_fRadius;
			layerHeight = cos(proportionSphere * i) * a_fRadius;

			// Calculate first points in new layers
			lastPointTop = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), layerHeight);
			lastPointBottom = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), -layerHeight);
			currentLayerTop[0] = lastPointTop;
			currentLayerBottom[0] = lastPointBottom;

			// Calculate layer of points
			for (int j = 1; j < a_nSubdivisions; j++)
			{
				// Calculate the current points of the subdivision
				currentPointTop = vector3(layerRadius * (cos(proportion * (j + 1))), layerRadius * (sin(proportion * (j + 1))), layerHeight);
				currentPointBottom = vector3(layerRadius * (cos(proportion * (j + 1))), layerRadius * (sin(proportion * (j + 1))), -layerHeight);

				// Add quad where
				//	a is the previously calculated point on the top
				//	b is the current calculated point on the top
				//	c is the previously indexed point on the top on the previous layer
				//	d is the current indexed point on the top on the previous layer
				AddQuad(lastPointTop, currentPointTop, previousLayerTop[j - 1], previousLayerTop[j]);

				// Add quad where
				//	a is the current calculated point on the bottom
				//	b is the previously calculated point on the bottom
				//	c is the currently indexed point on the bottom on the previous layer
				//	d is the previously indexed point on the bottom on the previous layer
				AddQuad(currentPointBottom, lastPointBottom, previousLayerBottom[j], previousLayerBottom[j - 1]);

				// Set the calculated points to the last points for the next triangles and layers
				lastPointTop = currentPointTop;
				currentLayerTop[j] = currentPointTop;
				lastPointBottom = currentPointBottom;
				currentLayerBottom[j] = currentPointBottom;
			}

			// Calculate final points
			currentPointTop = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), layerHeight);
			currentPointBottom = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), -layerHeight);

			// Finish layers with final quads
			AddQuad(lastPointTop, currentPointTop, previousLayerTop[a_nSubdivisions - 1], previousLayerTop[0]);
			AddQuad(currentPointBottom, lastPointBottom, previousLayerBottom[0], previousLayerBottom[a_nSubdivisions - 1]);

			// Set current layer to previous layer
			for (int j = 0; j < a_nSubdivisions; j++)
			{
				previousLayerTop[j] = currentLayerTop[j];
				previousLayerBottom[j] = currentLayerBottom[j];
			}
		}

		// Calculate last set of points
		// Calculate new layer height and radius
		layerRadius = sin(proportionSphere * (a_nSubdivisions / 2)) * a_fRadius;
		layerHeight = cos(proportionSphere * (a_nSubdivisions / 2)) * a_fRadius;

		// Calculate first points in new layers
		lastPointTop = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), layerHeight);
		lastPointBottom = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), -layerHeight);

		// Calculate layer of points
		for (int i = 1; i < a_nSubdivisions; i++)
		{
			// Calculate the current points of the subdivision
			currentPointTop = vector3(layerRadius * (cos(proportion * (i + 1))), layerRadius * (sin(proportion * (i + 1))), layerHeight);
			currentPointBottom = vector3(layerRadius * (cos(proportion * (i + 1))), layerRadius * (sin(proportion * (i + 1))), -layerHeight);

			// Add quad where
			//	a is the previously calculated point on the top
			//	b is the current calculated point on the top
			//	c is the previously indexed point on the top on the previous layer
			//	d is the currently indexed point on the top on the previous layer
			AddQuad(lastPointTop, currentPointTop, previousLayerTop[i - 1], previousLayerTop[i]);

			// Add quad where
			//	a is the current calculated point on the bottom
			//	b is the previously calculated point on the bottom
			//	c is the current indexed point on the bottom on the previous layer
			//	d is the previously indexed point on the bottom on the previous layer
			AddQuad(currentPointBottom, lastPointBottom, previousLayerBottom[i], previousLayerBottom[i - 1]);

			// Set the calculated points to the last points for the next triangles and layers
			lastPointTop = currentPointTop;
			lastPointBottom = currentPointBottom;
		}

		// Calculate final points
		currentPointTop = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), layerHeight);
		currentPointBottom = vector3(layerRadius * (cos(proportion)), layerRadius * (sin(proportion)), -layerHeight);

		// Finish layers with final quads
		AddQuad(lastPointTop, currentPointTop, previousLayerTop[a_nSubdivisions - 1], previousLayerTop[0]);
		AddQuad(currentPointBottom, lastPointBottom, previousLayerBottom[0], previousLayerBottom[a_nSubdivisions - 1]);
	}

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}