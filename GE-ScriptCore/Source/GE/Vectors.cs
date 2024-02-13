using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GE
{
    public struct Vector2
    {
        public float x, y;

        public Vector2(float scale)
        {
            this.x = scale;
            this.y = scale;
        }

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public static Vector2 Zero => new Vector2(0.0f);
        public static Vector2 operator *(Vector2 v, float scale)
        {
            return new Vector2(v.x * scale, v.y * scale);
        }

        public static Vector2 operator +(Vector2 v1, Vector2 v2)
        {
            return new Vector2(v1.x + v2.x, v1.y + v2.y);
        }
    }

    public struct Vector3
    {
        public float x, y, z;

        public Vector3(float scale)
        {
            this.x = scale;
            this.y = scale;
            this.z = scale;
        }

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public static Vector3 Zero => new Vector3(0.0f);
        public Vector2 XY => new Vector2(this.x, this.y);
        public static Vector3 operator *(Vector3 v, float scale)
        {
            return new Vector3(v.x * scale, v.y * scale, v.z * scale);
        }

        public static Vector3 operator +(Vector3 v1, Vector3 v2)
        {
            return new Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
        }
    }

}
