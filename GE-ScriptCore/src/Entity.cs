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

        public Entity FindEntityByName(string name)
        {
            ulong uuid = InternalCalls.Entity_FindEntityByName(name);
            if (uuid == 0)
            {
                GE.Log.LogCoreWarn("Cannot find Entity by Name. Returning null. UUID(" + uuid + ")");
                return null;
            }
            GE.Log.LogCoreInfo("Found Entity by Name. Returning Entity(" + uuid + ")");
            return new Entity(uuid);
        }

        public T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls.Entity_GetScriptInstance(ID);
            return instance as T;
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

        // Called when runtime Entity has an Event. See: Entity(cpp)
        public virtual bool OnEvent(Event.Type eType)
        {
            Log.LogCoreInfo("C# Base Entity Class OnEvent");
            return false;
        }

        protected virtual void OnCreate()
        {

        }

        protected virtual void OnUpdate(float ts)
        {

        }

    }
}
