﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using HdacSdk;
using Bitnet.Client;
using System.Net;
using Newtonsoft.Json.Linq;

namespace HdacSdkTest
{
    class Program
    {
        static void Main(string[] args)
        {
            //Keys.TestStruct result = new Keys.TestStruct();
            //Keys.test_return_mashal(ref result);
            //Console.WriteLine("result : {0}, {1}", result.ulongData, result.intData);
            //Console.WriteLine("result string : {0}, {1}", result.stringData1, result.stringData2);
            

            BitnetClient client = new BitnetClient("http://13.125.145.98:4260");
            client.Credentials = new NetworkCredential("hdacrpc", "1234");
            var p1 = client.GetBlockchainParams();

            Keys.PrivateKeyHelpInfo info = new Keys.PrivateKeyHelpInfo();
            info.addrChecksum = p1["address-checksum-value"].ToString();
            info.privateKeyPrefix = p1["private-key-version"].ToString();

            Keys.WalletAddrHelpInfo walletInfo = new Keys.WalletAddrHelpInfo();
            walletInfo.addrChecksum = p1["address-checksum-value"].ToString();
            walletInfo.pubKeyAddrPrefix = p1["address-pubkeyhash-version"].ToString();
            walletInfo.scriptAddrPrefix = p1["address-scripthash-version"].ToString();

            Keys.Keypairs keypairs = new Keys.Keypairs();
            Keys.create_key_pairs_shp(ref info, ref walletInfo, ref keypairs);

            Console.WriteLine("1. Test for creating key-pairs");
            Console.WriteLine("private key: {0}, \npublic key: {1}, \npublic hash: {2}, \nwallet addr: {3}", keypairs.privateKey, keypairs.pubKey, keypairs.pubKeyHash, keypairs.walletAddr);

            // wallet address "1WCRNaPb3jAjb4GE9t34uLiLtPseA8JKEvdtg5" is from the private key, "V6X4NaaDQSTgXdAcCzUrSxWqAuFcd53TRXRqmSafUYEbY5DgGMitPEzk"
            var unspents = client.ListUnspent("1WCRNaPb3jAjb4GE9t34uLiLtPseA8JKEvdtg5");
            JToken selected = null;
            foreach(var unspent in unspents)
            {
                if (unspent["assets"].Count() == 0)
                {
                    selected = unspent;
                }                
            }

            if (selected == null)
            {
                Console.WriteLine("unspent not founc");
                return;
            }
            
            string scriptPubKey = selected["scriptPubKey"].ToString();
            string txid = selected["txid"].ToString();
            uint vout = UInt32.Parse(selected["vout"].ToString());

            string createTxid = null;
            String streamName = "stream9";
            var streamInfos = client.ListStreams(streamName);
            foreach (var stream in streamInfos)
            {
                if (stream["name"].ToString() == streamName)
                {
                    createTxid = stream["createtxid"].ToString();
                }
            }
            
            if (createTxid == null)
            {
                Console.WriteLine("createTxid not found");
                return;
            }

            String checkRet = Keys.create_stream_publish_tx_shp("key1", "tested by moony",
                createTxid,
                scriptPubKey,
                txid, vout,
                "",
                "V6X4NaaDQSTgXdAcCzUrSxWqAuFcd53TRXRqmSafUYEbY5DgGMitPEzk", ref info);

            Console.WriteLine("\n2. Test for publishing stream");
            Console.WriteLine("raw-tx: {0}", checkRet);
        }
    }
}
