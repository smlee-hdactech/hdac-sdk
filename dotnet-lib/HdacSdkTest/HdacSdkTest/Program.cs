using System;
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

            
            checkRet = Keys.create_asset_send_tx_shp("1WCRNaPb3jAjb4GE9t34uLiLtPseA8JKEvdtg5", 10,
                                             "44fdb8103f4e13d6ef2011d54933f2747b455c613b3cfe4886d187330d50b640", 10,
                                             "76a9143ab53060d41b5fa662a2d4575a69464b5759839588ac1c73706b7174f23349d51120efd6134e3f10b8fd44ac2600000000000075",
                                             "030374d736a70c5faf5d16887d2263e812cb896938bedeefd44c128417e2460a", 1,
                                             990.0,
                                             "",
                                             "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp",
                                             ref info,
                                             ref walletInfo
                                             );
            Console.WriteLine("\n3. Test for create asset send tx");
            Console.WriteLine("raw-tx: {0}", checkRet);
            
            int Ret = Keys.verify_message_shp(
                        "18wD7MBodeTYRAvN5bRuWYB11jwHdkGVCBLSnB",
                        "IJKPyPUFEgnlrcixdqbfgAks89Gi29uzGAyMUYICz8VAWEs6VlOpjzregZ2WrcarZoNtXD7aLC2S6VWJ8XowH9c=",
                        "Hdac Technology, Solution Dev Team, Test Text.",
                        ref walletInfo);
            Console.WriteLine("\n4. Test for verify message");
            Console.Write("true(1) or false(0) [");
            Console.Write(Ret);
            Console.WriteLine("]");
            
            checkRet = Keys.sign_message_shp(
                        "VHXjccrTPdRXG8asyos5oqvw6mhWtqASkbFsVuBnkpi4WXn2jr8eMwwp",
                        "Hdac Technology, Solution Dev Team, Test Text.",
                        ref info, ref walletInfo);
            Console.WriteLine("\n5. Test for sign message");
            Console.Write("[");
            Console.Write(checkRet);
            Console.WriteLine("]");
        }
    }
}
