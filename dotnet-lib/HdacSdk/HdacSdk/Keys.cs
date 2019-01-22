using System;
using System.Runtime.InteropServices;

namespace HdacSdk
{
    public class Keys
    {
        [DllImport("keys_wrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        extern public static void test_char_param([MarshalAs(UnmanagedType.LPStr)]String streamKey);
                
        [DllImport("keys_wrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        extern public static void create_stream_publish_tx1([MarshalAs(UnmanagedType.LPStr)]String streamKey, [MarshalAs(UnmanagedType.LPStr)]String streamItem,
            [MarshalAs(UnmanagedType.LPStr)]String createTxid, [MarshalAs(UnmanagedType.LPStr)]String unspentScriptPubKey,
            [MarshalAs(UnmanagedType.LPStr)]String unspentTxid, uint unspentVOut,
            [MarshalAs(UnmanagedType.LPStr)]String unspentRedeemScript, [MarshalAs(UnmanagedType.LPStr)]String privateKey,
            [MarshalAs(UnmanagedType.LPStr)]String privateKeyPrefix, [MarshalAs(UnmanagedType.LPStr)]String addrChecksum);
    }
}
