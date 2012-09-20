using System;
using System.IO;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.GamerServices;
using System.Windows;

namespace MoSync
{
    public class MathModule : ISyscallModule, IIoctlModule
    {
			public void Init(Syscalls mSyscalls, Core mCore, Runtime mRuntime)
			{
			}

        public void Init(Ioctls ioctls, Core core, Runtime runtime)
        {
            ioctls.sinh = delegate(double d)
            {
                return BitConverter.DoubleToInt64Bits(Math.Sinh(d));
            };

            ioctls.cosh = delegate(double d)
            {
                return BitConverter.DoubleToInt64Bits(Math.Cosh(d));
            };

            ioctls.atanh = delegate(double d)
            {
                double value = (Math.Log(1.0 + d) - Math.Log(1.0 - d)) / 2.0;
                return BitConverter.DoubleToInt64Bits(value);
            };

        }
    }
}
