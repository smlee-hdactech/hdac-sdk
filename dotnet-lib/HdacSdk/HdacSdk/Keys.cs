using System;
using System.Runtime.InteropServices;

namespace HdacSdk
{
    public class Keys
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct PrivateKeyHelpInfo
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
            public String privateKeyPrefix;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
            public String addrChecksum;
        }

        [DllImport("keys_wrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        extern public static string create_stream_publish_tx_shp([MarshalAs(UnmanagedType.LPStr)]String streamKey, [MarshalAs(UnmanagedType.LPStr)]String streamItem,
            [MarshalAs(UnmanagedType.LPStr)]String createTxid, [MarshalAs(UnmanagedType.LPStr)]String unspentScriptPubKey,
            [MarshalAs(UnmanagedType.LPStr)]String unspentTxid, uint unspentVOut,
            [MarshalAs(UnmanagedType.LPStr)]String unspentRedeemScript, [MarshalAs(UnmanagedType.LPStr)]String privateKey,
            ref PrivateKeyHelpInfo info);

        [StructLayout(LayoutKind.Sequential)]
        public struct TestStruct
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
            public String stringData1;
            public readonly UInt64 ulongData;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 200)]
            public String stringData2;
            public readonly Int32 intData;
        }

        [DllImport("keys_wrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        extern public static void test_return_mashal(ref TestStruct result);
        //[return: MarshalAs(UnmanagedType.LPStruct)]
        //extern public static TestStruct test_return_mashal();
    }
}
