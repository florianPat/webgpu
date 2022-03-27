#include "Physics.h"
#include <cmath>
#include "Utils.h"
#include "RectangleShape.h"
#include "CircleShape.h"
#undef max
#include <limits>

void Physics::handleCollision(Body& itBody, Body& collideElementBody, const Collider & bodyCollider, const Collider& elementCollider, int32_t bodyIndex)
{
    assert(!itBody.isTrigger);
	if (collideElementBody.isTrigger)
	{
		if (bodyCollider.intersects(elementCollider))
		{
            collideElementBody.triggered = true;
            collideElementBody.triggerInformation.triggerElementCollision = itBody.id;
            collideElementBody.triggerInformation.index = bodyIndex;
		}

		return;
	}

	Vector3f minTransVec = {};
	if (bodyCollider.collide(elementCollider, &minTransVec))
	{
		if (minTransVec.x > 0.0f)
		{
			itBody.vel.x = 0;
			itBody.triggerInformation.triggerBodyPart = Body::TriggerBodyPart::LEFT;
		}
		else if (minTransVec.x < 0.0f)
		{
			itBody.vel.x = 0;
			itBody.triggerInformation.triggerBodyPart = Body::TriggerBodyPart::RIGHT;
		}
		if (minTransVec.y > 0.0f)
		{
			itBody.vel.y = 0;
			// TODO: Think about these body parts. Do they make sense in a "multiple collision" szenario?
            itBody.triggerInformation.triggerBodyPart = Body::TriggerBodyPart::SHOES;
		}
		else if (minTransVec.y < 0.0f)
		{
			itBody.vel.y = 0;
            itBody.triggerInformation.triggerBodyPart = Body::TriggerBodyPart::HEAD;
		}
		else if (minTransVec.z > 0.0f)
		{
			itBody.vel.z = 0;
			itBody.triggerInformation.triggerBodyPart = Body::TriggerBodyPart::BACK;
		}
		else if (minTransVec.z < 0.0f)
		{
			itBody.vel.z = 0;
			itBody.triggerInformation.triggerBodyPart = Body::TriggerBodyPart::FRONT;
		}

		itBody.pos += minTransVec;
	}
}

void Physics::Collider::getPointsAxis(Vector2f * points, Vector2f * axis) const
{
	switch (type)
	{
		case Collider::Type::obb:
		{
			OBB bodyOBB = collider.obb;

			points[0] = { bodyOBB.pos };
			points[1] = { bodyOBB.pos.x + bodyOBB.width, bodyOBB.pos.y };
			points[2] = { bodyOBB.pos.x + bodyOBB.width, bodyOBB.pos.y + bodyOBB.height };
			points[3] = { bodyOBB.pos.x, bodyOBB.pos.y + bodyOBB.height };

			//Global origin
			Vector2f origin = bodyOBB.pos + bodyOBB.origin;

			for (int32_t i = 0; i < 4; ++i)
			{
				points[i] = Vector2f(bodyOBB.pos + bodyOBB.xAxis * (points[i].x - origin.x) + bodyOBB.yAxis * (points[i].y - origin.y));
			}

			axis[0] = bodyOBB.xAxis;
			axis[1] = bodyOBB.yAxis;

			break;
		}
		case Collider::Type::rect:
		{
			FloatRect bodyRect = collider.rect;

			points[0] = { bodyRect.left, bodyRect.bottom };
			points[1] = { bodyRect.left + bodyRect.width, bodyRect.bottom };
			points[2] = { bodyRect.left + bodyRect.width, bodyRect.bottom + bodyRect.height };
			points[3] = { bodyRect.left, bodyRect.bottom + bodyRect.height };

			axis[0] = { 1.0f, 0.0f };
			axis[1] = { 0.0f, 1.0f };
			break;
		}
		case Collider::Type::circle:
		{
			FloatCircle bodyCircle = collider.circle;

			points[0] = bodyCircle.center;

			axis[0] = { 0.0f, 0.0f };
			axis[1] = { 0.0f, 0.0f };

			break;
		}
		default:
		{
			InvalidCodePath;
		}
	}
}

Vector2f Physics::Collider::getProjectionMinMax(const Vector2f * points, const Vector2f & axis, bool isXAxis) const
{
	Vector2f result = { points[0].x * axis.x + points[0].y * axis.y, points[0].x * axis.x + points[0].y * axis.y };

	if (type != Type::circle)
	{
		for (int32_t i = 1; i < 4; ++i)
		{
			float proj = points[i].x * axis.x + points[i].y * axis.y;

			if (proj < result.x)
				result.x = proj;
			else if (proj > result.y)
				result.y = proj;
		}
	}
	else
	{
		//TODO: Also, the circle goes a bit into the obb... (It happens because I just throw out the four corner points, but thats not really good for 
		// for example bottom left...
		//TODO: Use vohoo regions (or how its called) to make "corner collision" nicer
		float proj = (points[0].x + (isXAxis ? collider.circle.radius : 0)) * axis.x + (points[0].y + (isXAxis ? 0 : collider.circle.radius)) * axis.y;
		
		if (proj < result.x)
			result.x = proj;
		else if (proj > result.y)
			result.y = proj;


		proj = (points[0].x - (isXAxis ? collider.circle.radius : 0)) * axis.x + (points[0].y - (isXAxis ? 0 : collider.circle.radius)) * axis.y;

		if (proj < result.x)
			result.x = proj;
		else if (proj > result.y)
			result.y = proj;
	}

	return result;
}

Physics::Physics() : bodies()
{
	for(int32_t i = 0; i < NUM_LAYERS; ++i)
		collisionLayers[i] = 0;
}

void Physics::update(float dt)
{
	int32_t i = 0;
	//TODO: Add broad phase collision detection!
	for (auto it = bodies.begin(); it != bodies.end(); ++it, ++i)
	{
        if((it->isActive) && (!it->isTrigger) && (!it->isStatic))
        {
            Body& itBody = *it;
			itBody.triggerInformation.triggerBodyPart = Body::TriggerBodyPart::NONE;

            const Collider& bodyRect = itBody.physicsElements[0];

            int32_t oI = 0;
            for(auto collideLayerIt = it->collisionLayers.begin(); collideLayerIt != it->collisionLayers.end();
                ++collideLayerIt)
            {
                if(*collideLayerIt != 0)
                    oI = collisionLayers[*collideLayerIt - 1];

                auto collisionIdIt = bodies.begin() + oI;

                int32_t nextCollideLayer = collisionLayers[*collideLayerIt];

                for(; (oI < nextCollideLayer); ++collisionIdIt, ++oI)
                {
                    assert(collisionIdIt != bodies.end());

                    Body& collideElementBody = *collisionIdIt;

                    if(collideElementBody.isActive && oI != i)
                    {
                        for(const Collider& elementRect : collideElementBody.physicsElements)
                            handleCollision(itBody, collideElementBody, bodyRect, elementRect, i);
                    }
                }
            }

            it->pos += it->vel * dt;
        }
	}
}

void Physics::debugRenderBodies(Graphics2D& gfx) const
{
	for (auto it = bodies.begin(); it != bodies.end(); ++it)
	{
		const Collider& collider = it->physicsElements[0];

		RectangleShape body;

		switch (collider.type)
		{
			case Collider::Type::rect:
			{
				FloatRect colliderRect = collider.collider.rect;

				body.size = Vector2f(colliderRect.width, colliderRect.height);
				body.pos = Vector2f(colliderRect.left, colliderRect.bottom);
				body.fillColor = Colors::Yellow;

				gfx.draw(body);

				break;
			}
			case Collider::Type::obb:
			{
				OBB collideOBB = collider.collider.obb;

				body.pos = collideOBB.pos;
				body.size = Vector2f{ collideOBB.width, collideOBB.height };
				body.origin = collideOBB.origin;
				body.rotation = utils::radiansToDegrees(collideOBB.angle);
				body.fillColor = Colors::Yellow;
#if 0
				Vector2f points[4] = { { collideOBB.pos },{ collideOBB.pos.x + collideOBB.width, collideOBB.pos.y },
										 { collideOBB.pos.x + collideOBB.width, collideOBB.pos.y + collideOBB.height },
										 { collideOBB.pos.x, collideOBB.pos.y + collideOBB.height } };

				//Global origin
				Vector2f origin = collideOBB.pos + collideOBB.origin;

				for (int32_t i = 0; i < 4; ++i)
				{
					points[i] = Vector2f(collideOBB.pos + (points[i].x - origin.x) * collideOBB.xAxis + (points[i].y - origin.y) * collideOBB.yAxis);
				}

				for (uint32_t32_t i = 0; i < body.getPointCount(); ++i)
				{
					Vector2f myPoint = points[i];
					Vector2f point = body.getPoint(i);
					point = body.getTransform().transformPoint(point);
					sf::Transform transform = body.getTransform();

					std::cout << myPoint.x << " " << myPoint.y << "---" << point.x << " " << point.y << std::endl;
				}
#endif

				gfx.draw(body);

				break;
			}
			case Collider::Type::circle:
			{
				CircleShape body;

				FloatCircle circle = collider.collider.circle;

				body.pos = Vector2f(circle.center.x - circle.radius, circle.center.y - circle.radius);
				body.radius = circle.radius;
				body.fillColor = Colors::Yellow;

				gfx.draw(body);

				break;
			}
		}
	}
}

void Physics::debugRenderBodies(Graphics3D& gfx) const
{
	uint8_t buffer[] = { 252, 186, 3, 128 };
	Texture yellowTexture(buffer, 1, 1);

	for (auto it = bodies.begin(); it != bodies.end(); ++it)
	{
		const Collider& collider = it->physicsElements[0];

		switch (collider.type)
		{
			case Collider::Type::cube:
			{
				FloatCube colliderCube = collider.collider.cube;

				Model body = Model::cube();

				Vector3f scl = { colliderCube.width / 2.0f, colliderCube.height / 2.0f, colliderCube.depth / 2.0f };
				Vector3f pos = { colliderCube.getRight() - scl.x, colliderCube.getTop() - scl.y, colliderCube.getFront() - scl.z };
				Vector3f rot = { 0.0f, 0.0f, 0.0f };

				gfx.draw(body, pos, rot, scl, &yellowTexture);

				break;
			}
		}
	}
}

int32_t Physics::addElementPointer(Body&& body, int32_t layer)
{
	assert(body.physicsElements.size() == 1);

	addElementValue(std::move(body), layer);

	return (bodyIndices.size() - 1);
}

void Physics::addElementValue(Body&& body, int32_t layer)
{
	assert(layer < NUM_LAYERS);

	int32_t index = collisionLayers[layer]++;
	for(int32_t i = layer + 1; i < NUM_LAYERS; ++i)
	{
		++collisionLayers[i];
	}
	bodies.insert(index, std::move(body));
	for(uint32_t i = 0; i < bodyIndices.size(); ++i)
	{
		if(bodyIndices[i] >= index)
			++bodyIndices[i];
	}
	bodyIndices.push_back(index);
}

void Physics::removeElementByIndex(int32_t index)
{
	// TODO: Add free list for bodyIndicies
	int32_t realIndex = getRealIndex(index);
	bodies.erase(realIndex);

	for (int32_t i = 0; i < NUM_LAYERS; ++i)
	{
		if (collisionLayers[i] >= realIndex)
		{
			--collisionLayers[i];
		}
	}

	for (uint32_t i = 0; i < bodyIndices.size(); ++i)
	{
		if (bodyIndices[i] >= realIndex)
		{
			--bodyIndices[i];
		}
	}
}

#if 0
void Physics::applySpriteToBoundingBox(const Sprite & sprite, Collider & boundingBox)
{
	assert(boundingBox.type == Collider::Type::rect);

	boundingBox.collider.rect.left = sprite.getGlobalBounds().left;
	boundingBox.collider.rect.bottom = sprite.getGlobalBounds().bottom;
	boundingBox.collider.rect.width = (float)sprite.getGlobalBounds().width;
	boundingBox.collider.rect.height = (float)sprite.getGlobalBounds().height;
}
#endif

Vector<ShortString> Physics::getAllCollisionIdsWhichContain(const ShortString & string)
{
	Vector<ShortString> result;

	for (auto it = bodies.begin(); it != bodies.end(); ++it)
	{
		uint32_t match = it->id.find(string, 0);
		if (match != String::npos)
		{
			bool onlyNumbers = true;
			ShortString substr = it->id;
			substr.erase(match, string.length());
			for (auto it = substr.begin(); it != substr.end(); ++it)
			{
				char c = *it;
				//NOTE: has to be ascii!
				if (c >= 48 && c <= 57)
					continue;
				else
				{
					onlyNumbers = false;
					break;
				}
			}
			if (onlyNumbers)
				result.push_back(it->id);
		}
	}

	return result;
}

Physics::Body* Physics::getBodyFromRealIndex(int32_t realIndex)
{
    assert(realIndex >= 0 && (uint32_t)realIndex < bodies.size());

    return &bodies[realIndex];
}

int32_t Physics::getRealIndex(int32_t index) const
{
	assert(index >= 0 && (uint32_t)index < bodyIndices.size());

	return bodyIndices[index];
}

Physics::Body::Body(Vector3f&& pos, const ShortString& name, Collider&& collider, Vector<int32_t>&& collideLayers,
		bool isTrigger, bool isStatic)
	: isStatic(isStatic), isTrigger(isTrigger), id(name), physicsElements{}, collisionLayers(collideLayers), pos(pos)
{
	this->physicsElements.push_back(collider);

#ifdef DEBUG
	checkCollideLayers();
#endif
}

Physics::Body::Body(const ShortString& name, Collider&& collider, bool isTrigger, bool isStatic)
	: isStatic(isStatic), isTrigger(isTrigger), id(name), physicsElements{}, collisionLayers(), pos(0.0f, 0.0f, 0.0f)
{
	this->physicsElements.push_back(collider);
}

Physics::Body::Body(const ShortString& name, Vector<Collider>&& colliders, bool isTrigger)
	: isStatic(true), isTrigger(isTrigger), id(name), physicsElements(colliders), collisionLayers(), pos(0.0f, 0.0f, 0.0f)
{
}

bool Physics::Body::getIsTriggerd()
{
	bool result = triggered;
    triggered = false;
	return result;
}

const Physics::Body::TriggerInformation & Physics::Body::getTriggerInformation() const
{
	return triggerInformation;
}

const ShortString & Physics::Body::getId() const
{
	return id;
}

void Physics::Body::checkCollideLayers()
{
	if(!collisionLayers.empty())
	{
		for(uint32_t i = 0; i < this->collisionLayers.size() - 1; ++i)
		{
			assert(this->collisionLayers[i] < this->collisionLayers[i + 1]);
		}
	}
}

Physics::Collider& Physics::Body::getCollider()
{
    return physicsElements[0];
}

void Physics::Body::setIsActive(bool isActiveIn)
{
	isActive = isActiveIn;
	triggered = false;
}

bool Physics::Body::getIsActive() const
{
	return isActive;
}

Physics::Collider::Collider() : type(Type::rect), collider{ {} }
{
}

Physics::Collider::Collider(FloatRect & rect) : type(Type::rect), collider{ {rect.left, rect.bottom, rect.width, rect.height} }
{
}

Physics::Collider::Collider(OBB & obb) : type(Type::obb), collider{ {} }
{
	collider.obb = obb;
}

Physics::Collider::Collider(FloatCircle & circle) : type(Type::circle), collider{ {} }
{
	collider.circle = circle;
}

Physics::Collider::Collider(FloatCube& cube) : type(Type::cube), collider{ {} }
{
	collider.cube = cube;
}

Physics::Collider::Collider(FloatLine& line) : type(Type::line), collider{ {} }
{
	collider.line = line;
}

Physics::Collider::Collider(FloatRect && rect) : type(Type::rect), collider{ rect }
{
}

Physics::Collider::Collider(OBB && obb) : type(Type::obb), collider{ {} }
{
	collider.obb = obb;
}

Physics::Collider::Collider(FloatCircle && circle) : type(Type::circle), collider{ {} }
{
	collider.circle = circle;
}

Physics::Collider::Collider(FloatCube&& cube) : type(Type::cube), collider{ {} }
{
	collider.cube = cube;
}

Physics::Collider::Collider(FloatLine&& line) : type(Type::line), collider{ {} }
{
	collider.line = line;
}

Physics::Collider::Collider(Terrain* terrain) : type(Type::terrain), collider{ {} }
{
	collider.terrain = terrain;
}

Physics::Collider::Type Physics::Collider::GetType() const
{
	return type;
}

bool Physics::Collider::intersects(const Collider & other) const
{
	if (other.type == Type::rect && type == Type::rect)
	{
		return collider.rect.intersects(other.collider.rect);
	}
	else if (other.type == Type::cube && type == Type::cube)
	{
		return collider.cube.intersects(other.collider.cube);
	}
	else if (other.type == Type::line && type == Type::cube)
	{
		FloatCube cube = collider.cube;
		FloatLine line = other.collider.line;

		Vector3f corners[] = {
			{ cube.left, cube.bottom, cube.back },
			{ cube.getRight(), cube.getTop(), cube.getFront() }
		};

		Vector3f lineVec = line.p1 - line.p0;

		float lineVecArray[] = { lineVec.x, lineVec.y, lineVec.z };
		float corner0Array[] = { corners[0].x, corners[0].y, corners[0].z };
		float corner1Array[] = { corners[1].x, corners[1].y, corners[1].z };
		for (uint32_t i = 0; i < arrayCount(lineVecArray); ++i)
		{
			if (lineVecArray[i] == 0.0f)
			{
				if (lineVecArray[i] < corner0Array[i] || lineVecArray[i] > corner1Array[i])
				{
					return false;
				}
			}
		}

		Vector3f minVec = (corners[0] - line.p0) / lineVec;
		Vector3f maxVec = (corners[1] - line.p0) / lineVec;

		float min = 0.0f;
		float max = 1.0f;
		float mins[] = { minVec.x, minVec.y, minVec.z };
		float maxs[] = { maxVec.x, maxVec.y, maxVec.z };

		for (uint32_t i = 0; i < 3; ++i)
		{
			if (mins[i] > maxs[i])
			{
				float temp = maxs[i];
				maxs[i] = mins[i];
				mins[i] = temp;
			}
			if (mins[i] > min)
			{
				min = mins[i];
			}
			if (maxs[i] < max)
			{
				max = maxs[i];
			}
			if (min > max)
			{
				return false;
			}
		}
		return true;
	}
	else if (other.type == Type::circle && type == Type::circle)
	{
		Vector2f vec = collider.circle.center - other.collider.circle.center;
		return (sqrtf(vec.x * vec.x + vec.y * vec.y) < collider.circle.radius + other.collider.circle.radius);
	}
	else if (other.type == Type::terrain && type == Type::cube)
	{
		return collider.cube.bottom < (other.collider.terrain->getHeight(collider.cube.left, collider.cube.back) + 5.0f);
	}
	else
	{
		assert(type != Type::terrain && other.type != Type::terrain);

		Vector2f axis[4] = {};

		Vector2f s1Points[4] = {};
		Vector2f s2Points[4] = {};

		getPointsAxis(s1Points, axis);
		other.getPointsAxis(s2Points, axis + 2);

		for (int32_t i = 0; i < 4; ++i)
		{
			Vector2f s1MinMax = getProjectionMinMax(s1Points, axis[i], i % 2 == 0);
			Vector2f s2MinMax = getProjectionMinMax(s2Points, axis[i], i % 2 == 0);

			if ((s2MinMax.x > s1MinMax.y || s2MinMax.y < s1MinMax.x))
				return false;
			else
			{
				continue;
			}
		}

		return true;
	}
}

bool Physics::Collider::collide(const Collider & other, Vector3f *minTransVec) const
{
	if (other.type == Type::rect && type == Type::rect)
	{
		bool result = collider.rect.intersects(other.collider.rect);
		
		if (result)
		{
			FloatRect rect = collider.rect;
			FloatRect otherRect = other.collider.rect;
			
			*minTransVec = { rect.left - (otherRect.left + otherRect.width), 0, 0 };
			Vector3f corners[3];
			corners[0] = Vector3f{ (rect.left + rect.width) - otherRect.left, 0, 0 };
			corners[1] = Vector3f{ 0, rect.bottom - (otherRect.bottom + otherRect.height), 0 };
			corners[2] = Vector3f{ 0, (rect.bottom + rect.height) - otherRect.bottom, 0 };

			for (uint32_t i = 0; i < arrayCount(corners); ++i)
			{
			    Vector3f* it = &corners[i];
				if (fabsf(minTransVec->x * minTransVec->x + minTransVec->y * minTransVec->y) > fabsf(it->x * it->x + it->y * it->y))
				{
					*minTransVec = -*it;
				}
			}
		}

		return result;
	}
	else if (other.type == Type::cube && type == Type::cube)
	{
		bool result = collider.cube.intersects(other.collider.cube);

		if (result)
		{
			FloatCube cube = collider.cube;
			FloatCube otherCube = other.collider.cube;

			*minTransVec = { cube.left - otherCube.getRight(), 0, 0 };
			Vector3f corners[5];
			corners[0] = Vector3f{ cube.getRight() - otherCube.left, 0, 0 };
			corners[1] = Vector3f{ 0, cube.bottom - otherCube.getTop(), 0 };
			corners[2] = Vector3f{ 0, cube.getTop() - otherCube.bottom, 0 };
			corners[3] = Vector3f{ 0, 0, cube.back - otherCube.getFront() };
			corners[4] = Vector3f{ 0, 0, cube.getFront() - otherCube.back };

			for (uint32_t i = 0; i < arrayCount(corners); ++i)
			{
				Vector3f* it = &corners[i];
				if (fabsf(minTransVec->x * minTransVec->x + minTransVec->y * minTransVec->y + minTransVec->z * minTransVec->z)
					>
					fabsf(it->x * it->x + it->y * it->y + it->z * it->z))
				{
					*minTransVec = -*it;
				}
			}
		}

		return result;
	}
	else if (other.type == Type::circle && type == Type::circle)
	{
		Vector2f vec = collider.circle.center - other.collider.circle.center;
		float lenght = sqrtf(vec.x * vec.x + vec.y * vec.y);
		if (lenght < collider.circle.radius + other.collider.circle.radius)
		{
			Vector3f normalizedVec = { vec.x / lenght, vec.y / lenght, 0 };
			float overlap = lenght - (collider.circle.radius + other.collider.circle.radius);
			*minTransVec = -(normalizedVec * overlap);

			return true;
		}
		else
			return false;
	}
	else if (other.type == Type::terrain && type == Type::cube)
	{
		if (intersects(other))
		{
			*minTransVec = { 0.0f, other.collider.terrain->getHeight(collider.cube.left, collider.cube.back) - collider.cube.bottom + 5.0f, 0.0f };
			return true;
		}

		return false;
	}
	else
	{
		assert(other.type != Type::cube && type != Type::cube && other.type != Type::terrain && type != Type::terrain);

		//NOTE: xAxis goes first!!
		Vector2f axis[4] = {};

		Vector2f s1Points[4] = {};
		Vector2f s2Points[4] = {};

		float angle = 0.0f;

		float o = std::numeric_limits<float>::max();
		Vector2f minAxis = { 0.0f, 0.0f };

		//Get x and y axis, and the points of the collider
		getPointsAxis(s1Points, axis);
		other.getPointsAxis(s2Points, axis + 2);

		for (int32_t i = 0; i < 4; ++i)
		{
			if (axis[i].x == 0.0f && axis[i].y == 0.0f)
				continue;

			//Project points on axis with dot-product
			Vector2f s1MinMax = getProjectionMinMax(s1Points, axis[i], i % 2 == 0);
			Vector2f s2MinMax = other.getProjectionMinMax(s2Points, axis[i], i % 2 == 0);

			//Check for 1d-intersection (all axis have to intersect if the colliders collide)
			if ((s2MinMax.x > s1MinMax.y || s2MinMax.y < s1MinMax.x))
				return false;
			else
			{
				float overlap = s1MinMax.y > s2MinMax.y ? -(s2MinMax.y - s1MinMax.x) : (s1MinMax.y - s2MinMax.x);
				if (fabsf(overlap) < fabsf(o))
				{
					o = overlap;
					minAxis = axis[i];
					if (i < 2)
					{
						if (type == Type::obb)
							angle = collider.obb.angle;
						else
							angle = 0.0f;
					}
					else
					{
						if (other.type == Type::obb)
							angle = other.collider.obb.angle;
						else
							angle = 0.0f;
					}
				}
			}
		}

		*minTransVec = -Vector3f(o * minAxis.x, o * minAxis.y, 0);

		return true;
	}
}

//NOTE: angle from degrees in radians, because cosf uses radians, but in matrix of SFML in Shape it uses degrees, so you have to convert back and forth...
Physics::OBB::OBB(float left, float top, float width, float height, float angle) : angle(utils::degreesToRadians(angle)), 
																				   xAxis(cosf(this->angle), sinf(this->angle)), 
																				   yAxis((-sinf(this->angle)), cosf(this->angle)),
																				   width(width), height(height), pos(Vector2f{ left, top }), origin(0.0f, 0.0f)
{
}

Physics::OBB::OBB(Vector2f && topLeft, float width, float height, float angle) : angle(utils::degreesToRadians(angle)),
																				 xAxis(cosf(this->angle), sinf(this->angle)), 
																				 yAxis((-sinf(this->angle)), cosf(this->angle)),
																				 width(width), height(height), pos(topLeft), origin(0.0f, 0.0f)
{
}

Physics::OBB::OBB(float left, float top, float width, float height, float angle, Vector2f&& origin) : angle(utils::degreesToRadians(angle)), 
																									xAxis(cosf(this->angle), sinf(this->angle)), 
																									yAxis((-sinf(this->angle)), cosf(this->angle)),
																									width(width), height(height), pos(Vector2f{ left, top }), 
																									origin(origin)
{
}

Physics::OBB::OBB(Vector2f && topLeft, float width, float height, float angle, Vector2f&& origin) : angle(utils::degreesToRadians(angle)),
																								 xAxis(cosf(this->angle), sinf(this->angle)), 
																								 yAxis((-sinf(this->angle)), cosf(this->angle)),
																								 width(width), height(height), pos(topLeft),
																								 origin(origin)
{
}

void Physics::OBB::setAngle(float newAngle)
{
	angle = utils::degreesToRadians(newAngle);
	xAxis = Vector2f(cosf(angle), sinf(angle));
	yAxis = Vector2f(-sinf(angle), cosf(angle));
}

float Physics::OBB::getAngle() const
{
	return utils::radiansToDegrees(angle);
}

Physics::FloatCircle::FloatCircle(Vector2f && center, float radius) : radius(radius), center(center)
{
}

Physics::FloatCircle::FloatCircle(float centerX, float centerY, float radius) : radius(radius), center(centerX, centerY)
{
}
