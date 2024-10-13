using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GE
{
    public class Event
    {
        public enum Type
        {
            None,
            WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
            AppTick, AppUpdate, AppRender,
            KeyPressed, KeyReleased, KeyTyped,
            MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
        }

        // TODO: Get Type from C++ using Type enum. Event::Type should be formated the same in C# & C++
        public Type GetEventType() { return Type.None; }
    }
}
