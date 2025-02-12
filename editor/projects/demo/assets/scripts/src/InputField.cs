using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GE;

namespace demo
{
    public class InputField : Entity
    {
        private Player player;

        private GUIInputFieldComponent inputFieldComponent;
        private GUICanvasComponent parentCanvas;

        protected override void OnCreate()
        {
            inputFieldComponent = GetComponent<GUIInputFieldComponent>();
            parentCanvas = Entity.FindEntityByName("Canvas").GetComponent<GUICanvasComponent>();
            player = Entity.FindEntityByName("Player").As<Player>();
        }

        protected override void OnUpdate(float ts)
        {
            if (inputFieldComponent == null)
                inputFieldComponent = GetComponent<GUIInputFieldComponent>();

            if (parentCanvas == null)
                parentCanvas = Entity.FindEntityByName("Canvas").GetComponent<GUICanvasComponent>();

            if (player == null)
                player = Entity.FindEntityByName("Player").As<Player>();

            if (parentCanvas != null)
            {
                if (Input.IsKeyDown(KeyCode.KEY_TAB))
                {
                    Log.LogCoreInfo("Showing mouse");
                    parentCanvas.ShowMouse = true;
                }
                else if (Input.IsKeyDown(KeyCode.KEY_BACKSPACE))
                {
                    Log.LogCoreInfo("Not showing mouse");
                    parentCanvas.ShowMouse = false;
                }
            }
        }

        protected override bool OnEvent(Event e)
        {
            return base.OnEvent(e);
        }
        protected override bool OnMousePressed(Event e, MouseCode mouseCode)
        {
            return false;
        }

        protected override bool OnKeyPressed(Event e, KeyCode keyCode)
        {
            return false;
        }
    }
}
