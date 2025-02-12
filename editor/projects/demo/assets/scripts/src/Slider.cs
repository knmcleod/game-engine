using GE;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace demo
{
    public class Slider : Entity
    {
        private Player player;

        private GUISliderComponent sliderComponent;

        protected override void OnCreate()
        {
            sliderComponent = GetComponent<GUISliderComponent>();
            player = Entity.FindEntityByName("Player").As<Player>();
        }

        protected override void OnUpdate(float ts)
        {
            if (player == null)
                player = Entity.FindEntityByName("Player").As<Player>();

            if (sliderComponent == null)
                sliderComponent = GetComponent<GUISliderComponent>();
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
            if (player != null && sliderComponent != null)
            {
                // TODO : Direction
                if (keyCode.Equals(KeyCode.KEY_LEFT))
                    player.Speed += sliderComponent.Fill;
                else if (keyCode.Equals(KeyCode.KEY_RIGHT))
                    player.Speed -= 1.0f - sliderComponent.Fill;
            }
            return false;
        }
    }
}
