using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using HdacSdk;

namespace HdacSdkTest
{
    class Program
    {
        static void Main(string[] args)
        {
            Keys.PrivateKeyHelpInfo info = new Keys.PrivateKeyHelpInfo();
            info.addrChecksum = "cb507245";
            info.privateKeyPrefix = "8075fa23";

            String checkRet = Keys.create_stream_publish_tx_shp("key1", "tested by moony",
                "a0b59e8c6f2fd144485d19632f62708f88116fb11a46411dd7d1e211ec92ce9a",
                "a9143e45d3a48882576ad5900978303705e1a6000305871473706b6511feed9499be6fb101e0f59119d3fe15751473706b700800000000000000fffffffffbfe095c75",
                "db84077722b74c9c9a799cf58d6c7a265f214f003b5ef15cae368a8add8d33f8", 0,
                "5221027e75736b41474547b7e2443d7235f4030cbb378093bbd2e98ea36ded6d703c2b21038d7724f227aab828d771eb6ab697f333e615d39b585944d99737ce7b7ae650fd52ae",
                "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp", ref info);

            Console.WriteLine("raw-tx: {0}", checkRet);
        }
    }
}
