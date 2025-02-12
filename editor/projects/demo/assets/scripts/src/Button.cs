using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GE;

namespace demo
{
    public class Button : Entity
    {
        private Player player;

        private GUIButtonComponent buttonComponent;

        protected override void OnCreate()
        {
            buttonComponent = GetComponent<GUIButtonComponent>();
            player = Entity.FindEntityByName("Player").As<Player>();
        }

        protected override void OnUpdate(float ts)
        {
            if (buttonComponent == null)
                buttonComponent = GetComponent<GUIButtonComponent>();

            if (player == null)
                player = Entity.FindEntityByName("Player").As<Player>();
        }
        protected override bool OnEvent(Event e)
        {
            return base.OnEvent(e);
        }

        protected override bool OnMousePressed(Event e, MouseCode mouseCode)
        {
            if (buttonComponent != null && player != null)
            {
                player.Color = buttonComponent.HoveredColor;
                return true;
            }
            return false;
        }

        protected override bool OnKeyPressed(Event e, KeyCode keyCode)
        {
            if (buttonComponent != null && player != null)
            {
                player.Color = buttonComponent.HoveredColor;
                return true;
            }
            return false;
        }
    }
}
