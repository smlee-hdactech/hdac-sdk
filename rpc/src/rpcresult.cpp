#include "rpcresult.h"
#include <json_spirit/json_spirit.h>
#include <json_spirit/json_spirit_reader_template.h>

using namespace std;
using namespace json_spirit;

vector<string> findItemsFromRpcResult(const string& rpcResult, const vector<string>& keys) {
    vector<string> resultItems;

    Value resultValue;
    if (read_string(rpcResult, resultValue))   {
        for (const string& key : keys) {
            const Value &item = find_value(resultValue.get_obj(), key);
            //cout << key << ": " << item.get_str() << endl;
            resultItems.push_back(item.get_str());
        }
    }
    return resultItems;
}

map<string, string> mapFromRpcResult(const string& rpcResult, const vector<string>& keys) {
    map<string, string> resultItems;

    Value resultValue;
    if (read_string(rpcResult, resultValue))   {
        for (const string& key : keys) {
            const Value &item = find_value(resultValue.get_obj(), key);
            //cout << key << ": " << item.get_str() << endl;
            resultItems[key] = item.get_str();
        }
    }
    return resultItems;
}
