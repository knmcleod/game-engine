using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GE
{
    public class Event
    {
        // Formated the same in C# & C++
        public enum Type
        {
            None,
            WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
            AppTick, AppUpdate, AppRender,
            KeyPressed, KeyReleased, KeyTyped,
            MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
        }

        public Event(bool isHandled, Type type)
        {
            this.isHandled = isHandled;
            this.type = type;
        }

        protected bool isHandled;
        protected Type type;
    }

}
