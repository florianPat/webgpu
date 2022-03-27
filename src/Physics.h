#pragma once

#include "Vector2.h"
#include "Vector.h"
#include "Rect.h"
#include "Graphics.h"
#include "Cube.h"
#include "Terrain.h"

class Physics
{
public:
	struct FloatCircle
	{
		float radius = 0.0f;
		Vector2f center;
	public:
		FloatCircle(Vector2f&& center, float radius);
		FloatCircle(float centerX, float centerY, float radius);
		FloatCircle() = default;
	};

	class OBB
	{
	private:
		friend class Physics;

		float angle = 0.0f;
		Vector2f xAxis, yAxis;
	public:
		float width = 0.0f, height = 0.0f;
		Vector2f pos;
		Vector2f origin;
	public:
		//angle has to be in degrees!
		OBB(float left, float top, float width, float height, float angle);
		OBB(Vector2f&& topLeft, float width, float height, float angle);
		//Local origin
		OBB(float left, float top, float width, float height, float angle, Vector2f&& origin);
		OBB(Vector2f&& topLeft, float width, float height, float angle, Vector2f&& origin);
		//angle has to be in degrees!
		void setAngle(float newAngle);
		float getAngle() const;
		OBB() = default;
	};

	struct FloatLine
	{
		Vector3f p0, p1;
	};

	class Collider
	{
	public:
		enum class Type
		{
			rect,
			obb,
			circle,
			cube,
			line,
			terrain
		};
	private:
		friend class Physics;

		Type type;
	public:
		union
		{
			FloatRect rect;
			OBB obb;
			FloatCircle circle;
			FloatCube cube;
			FloatLine line;
			Terrain* terrain;
		} collider;

		Collider(FloatRect& rect);
		Collider(OBB& obb);
		Collider(FloatCircle& circle);
		Collider(FloatCube& cube);
		Collider(FloatLine& line);

		Collider(FloatRect&& rect);
		Collider(OBB&& obb);
		Collider(FloatCircle&& circle);
		Collider(FloatCube&& cube);
		Collider(FloatLine&& line);

		Collider(Terrain* terrain);

		Type GetType() const;
	private:
		Collider();

		bool intersects(const Collider& other) const;
		bool collide(const Collider& other, Vector3f* minTransVec) const;

		void getPointsAxis(Vector2f* points, Vector2f* axis) const;
		Vector2f getProjectionMinMax(const Vector2f* points, const Vector2f& axis, bool isXAxis) const;
	};
public:
	class Body
	{
		friend class Physics;
	public:
		enum class TriggerBodyPart
		{
			NONE,
			HEAD,
			SHOES,
			LEFT,
			RIGHT,
			BACK,
			FRONT
		};
	private:
		struct TriggerInformation
		{
			ShortString triggerElementCollision = "";
			int32_t index = -1;
			TriggerBodyPart triggerBodyPart = TriggerBodyPart::NONE;
		};

		bool isStatic;
		bool isTrigger;
		bool triggered = false;
		bool isActive = true;
    private:
		TriggerInformation triggerInformation = {};
		ShortString id;
		Vector<Collider> physicsElements;
		Vector<int32_t> collisionLayers;
	public:
		Vector3f pos;
		Vector3f vel = { 0.0f, 0.0f, 0.0f };
		bool usesGravity = true;
	public:
		//Should be called, if the object is moving
		Body(Vector3f&& pos, const ShortString& name, Collider&& collider, Vector<int32_t>&& collideLayers,
				bool isTrigger = false, bool isStatic = false);
		//Should be called if the object, is a static one
		Body(const ShortString& name, Collider&& collider, bool isTrigger = false, bool isStatic = true);
		//To have one name for a lot of Colliders. The body you have to pass by value, because pos and that does not make sense to manipulate here!
		Body(const ShortString& name, Vector<Collider>&& colliders, bool isTrigger = false);
	public:
		bool getIsTriggerd();
		const TriggerInformation& getTriggerInformation() const;
		const ShortString& getId() const;
		Collider& getCollider();
		void setIsActive(bool isActive);
		bool getIsActive() const;
	private:
		void checkCollideLayers();
	};
public:
	static constexpr float GRAVITY = -9.81f;
private:
	static constexpr int32_t NUM_LAYERS = 5;
	Vector<Body> bodies;
	int32_t collisionLayers[NUM_LAYERS];
	//TODO: Just call an event whenever a new entrie gets created! Maybe that ditches frame time but: try and see :)
	Vector<int32_t> bodyIndices;
private:
	void handleCollision(Body& itBody, Body& collideElementBody, const Collider & bodyCollider, const Collider& elementCollider, int32_t bodyIndex);
public:
	Physics();
	void update(float dt);
	void debugRenderBodies(Graphics2D& gfx) const;
	void debugRenderBodies(Graphics3D& gfx) const;
	//Use if you need a reference to the body, to get back triggerInformation etc.
	int32_t addElementPointer(Body&& body, int32_t layer);
	//You need to call this each frame, because the Body could be "moved" to another memory location
	int32_t getRealIndex(int32_t index) const;
	Body* getBodyFromRealIndex(int32_t realIndex);
	//Use this otherwise
	void addElementValue(Body&& body, int32_t layer);
	void removeElementByIndex(int32_t realIndex);
	Vector<ShortString> getAllCollisionIdsWhichContain(const ShortString & string);
};