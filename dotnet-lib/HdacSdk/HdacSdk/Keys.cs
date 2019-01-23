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
    }
}
