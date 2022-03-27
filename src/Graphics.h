#pragma once

#include "GraphicsVKIniter.h"
#include "GraphicsVK2D.h"
#include "GraphicsVK3D.h"

#define Graphics2D GraphicsVK2D
#define Graphics3D GraphicsVK3D
#define GraphicsIniter GraphicsVKIniter

struct InstancedInfo
{
	Vector3f pos;
	Vector3f rot;
	Vector3f scl;
};