#pragma once

class Entity;

//! Checks an entity's component signature against all registered systems.
//! Called whenever components are added or removed from an entity.
void CheckAgainstAllSystems(Entity* entity);
