# Client PACS primitives lifecycle

The client owns a single global PACS move container, `PACS` (`ryzom/client/src/pacs_client.cpp`). Several client objects keep raw `UMovePrimitive *` handles allocated from it. A handle must never outlive the container that allocated it: primitives are plain `new`/`delete` objects (`CMoveContainer::allocatePrimitive`/`freePrimitive`), so a stale handle points to freed memory, whose address may already be reused by a live primitive of a newer container.

## Container creation, destruction and hibernation

| Operation | Location |
|---|---|
| Creation | `initPACS()` (`pacs_client.cpp`) |
| Destruction | `releasePACS()` (`pacs_client.cpp`), the only call to `UMoveContainer::deleteMoveContainer` in the client. Called by `CContinent::select()` and `CContinent::unselect()` |
| Hibernation | `std::swap(PACS, PACSHibernated)` in `CContinentManager` (`continent_manager.cpp`), when entering an indoor continent and in `reset()`. The container is kept alive but is no longer the current one |

## Handle holders

| Holder | Created in | Cleared before container destruction by |
|---|---|---|
| `CEntityCL::_Primitive` | `CEntityCL::initPrimitive()` (`entity_cl.cpp`) | `releasePACS()` → `CEntityManager::removeCollision()` |
| `CUserEntity::_CheckPrimitive` | `CUserEntity::checkPos()` (`user_entity.cpp`) | `releasePACS()` → `CEntityManager::removeCollision()` (virtual `CUserEntity::removePrimitive()`) |
| `CShapeInstanceReference::Primitive` (`CEntityManager::_ShapeInstances`) | `CEntityManager::createInstance()` (`entities.cpp`) | `releasePACS()` → `CEntityManager::removeShapePrimitives()`, IG zone shapes included |
| `COutpost::_AddedPrims` | `outpost.cpp` | `CContinent::unselect()` → `removeOutpost()` |
| IG collision primitives | `IGCallbacks` (`ig_callback.cpp`) | `CContinent::unselect()` → `IGCallbacks->resetContainer()` |
| `CDoorManager::SDoor::Prims` | `CDoorManager::loadedCallback()` (`door_manager.cpp`) | `CDoorManager::removedCallback()` when the door's instance group is removed |

Entity primitives are recreated in the new container by `CEntityManager::changeContinent()` → `computePrimitive()`. Shape primitives are not recreated: a shape that survives the destruction of its container keeps its 3D instance but has no collision anymore.

## Known limitation: hibernation

On hibernation, entity primitives are removed (`CEntityManager::removeCollision()` in `CContinentManager::select()`), but shape primitives stay in the hibernated container, so that shapes keep their collision when the hibernated continent becomes current again. While another container is current, removing such a shape calls `removePrimitive()` on a container that does not own the primitive: `CMoveContainer::removePrimitive()` ignores any primitive absent from its `_PrimitiveSet` and logs a warning.
