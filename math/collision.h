#pragma once

class Vector;
class AABB;

bool LineAABBIntersection(const AABB& aabbBox, const Vector& v0, const Vector& v1, Vector& vecIntersection, float& flFraction);
