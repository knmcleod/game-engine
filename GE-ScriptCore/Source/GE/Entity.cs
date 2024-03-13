using System;

namespace GE
{
    public class Entity
    {
        protected Entity() { ID = 0; }
        internal Entity(ulong uuid)
        {
            ID = uuid;
        }

        public readonly ulong ID;

        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.TransformComponent_SetTranslation(ID, ref value);
            }
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return InternalCalls.Entity_HasComponent(ID, componentType);
        }
        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this };
            return component;
        }

        public Entity FindEntityByTag(string tag)
        {
            ulong uuid = InternalCalls.Entity_FindEntityByTag(tag);
            if(uuid == 0)
                return null;

            return new Entity(uuid);
        }

        public T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls.Entity_GetScriptInstance(ID);
            return instance as T;    
        }
    }
}
