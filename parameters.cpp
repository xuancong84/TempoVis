#include "StdAfx.h"

#include "parameters.h"

const int	BELT_INC = (int)((FLOAT)TempoPrecision / BELT_FPS + 0.5);
const FLOAT BELT_OFF = (FLOAT)M_PI / BELTCIRCUMSIZE;

int total_added = 0;

GLfloat	space_attn[3] = { 0.0f, 0.2f, 0 };
float	zero_vector[4] = { 0, 0, 0, 1 };
float	ones_vector[4] = { 1, 1, 1, 1 };
float	zero_4vector[4] = { 0, 0, 0, 0 };

// Lighting and material parameters
FLOAT	l_ambient[4] = { 0.4f, 0.4f, 0.4f, 1.0f };
FLOAT	l_diffuse[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
FLOAT	l_specular[4] = { 1.5f, 1.5f, 1.5f, 1.0f };
FLOAT	m_ambient[4] = { 0.4f, 0.4f, 0.4f, 1.0f };
FLOAT	m_diffuse[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
FLOAT	m_specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
FLOAT	m_emissive[4] = { 0.01f, 0.01f, 0.01f, 1.0f };

