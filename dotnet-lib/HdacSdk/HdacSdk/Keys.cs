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

        [StructLayout(LayoutKind.Sequential)]
        public struct Keypairs
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
            public String privateKey;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
            public String pubKey;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
            public String pubKeyHash;

            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 100)]
            public String walletAddr;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct WalletAddrHelpInfo
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
            public String pubKeyAddrPrefix;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
            public String scriptAddrPrefix;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 10)]
            public String addrChecksum;
        }

        [DllImport("keys_wrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        extern public static void create_key_pairs_shp(ref PrivateKeyHelpInfo privatehelper,
			ref WalletAddrHelpInfo addrhelper, ref Keypairs result);
    }
}
