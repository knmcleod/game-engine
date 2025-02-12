using System;

namespace GE
{
    public class Entity
    {
        static public Entity FindEntityByName(string name)
        {
            ulong uuid = InternalCalls.Entity_FindEntityByName(name);
            if (uuid == 0)
            {
                GE.Log.LogCoreWarn("Cannot find Entity by Name. Returning null. Name(" + name + "). UUID(" + uuid + ")");
                return null;
            }
            GE.Log.LogCoreInfo("Found Entity by Name(" + name + "). Returning Entity(" + uuid + ")");
            return new Entity(uuid);
        }

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
 
        protected virtual void OnCreate()
        {
           
        }

        protected virtual void OnUpdate(float ts)
        {
            
        }

        protected virtual bool OnEvent(Event e)
        {
            /*
            switch (type)
            {
                case Event.Type.MouseMoved:
                    return OnMouseMoved(isHandled, type, Vector2.Zero); // TODO : Use current mouse position instead
                case Event.Type.MouseButtonPressed:
                    return OnMousePressed(isHandled, type, MouseCode.MOUSE_BUTTON_1); // TODO : Use current mouse button instead
                case Event.Type.MouseButtonReleased:
                    return OnMouseReleased(isHandled, type, MouseCode.MOUSE_BUTTON_1); // TODO : Use current mouse button instead
                case Event.Type.MouseScrolled:
                    return OnMouseScrolled(isHandled, type, Vector2.Zero); // TODO : Use current mouse instead
                case Event.Type.KeyPressed:
                    return OnKeyPressed(isHandled, type, KeyCode.KEY_0); // TODO : Use current key instead
            }
            */
            Log.LogCoreTrace("Entity.OnEvent(Event)");
            return false;
        }

        protected virtual bool OnMouseMoved(Event e, Vector2 position)
        {
            return false;
        }

        protected virtual bool OnMousePressed(Event e, MouseCode mouseCode)
        {
            return false;
        }

        protected virtual bool OnMouseReleased(Event e, MouseCode mouseCode)
        {
            return false;
        }

        protected virtual bool OnMouseScrolled(Event e, Vector2 offset)
        {
            return false;
        }

        protected virtual bool OnKeyPressed(Event e, KeyCode keyCode)
        {
            return false;
        }


    }
}
