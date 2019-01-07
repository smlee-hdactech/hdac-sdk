#ifndef HS_KEYSHELPERWITHFILE_H
#define HS_KEYSHELPERWITHFILE_H

#include <memory>
#include <keys/keyshelper.h>
#include <map>

#ifndef IS_SPACE
#define IS_SPACE(x) (x == ' ' || x == '\n' || x == '\r' || x == '\t' || x == '\v')
#endif

class KeysHelperWithFileAll {
	public:
		KeysHelperWithFileAll(const std::string Path);

        const IWalletAddrHelper& addrHelper() const;

        const IPrivateKeyHelper& privHelper() const;

	private:
		std::map<std::string, std::string> _resultMap;

		class WalletAddrHelper : public IWalletAddrHelper {
			public:
				WalletAddrHelper(const std::map<std::string, std::string> &result) :
					_resultMap(result)  { }

				const std::vector<unsigned char> pubkeyAddrPrefix() const override;
				const std::vector<unsigned char> scriptAddrPrefix() const override;

				int32_t addrChecksumValue() const override;
			private:
				const std::map<std::string, std::string> & _resultMap;

		};
		std::unique_ptr<WalletAddrHelper> _addrHelper;

		class PrivateKeyHelper : public IPrivateKeyHelper {
			public:
				PrivateKeyHelper(const std::map<std::string, std::string> &result) :
					_resultMap(result)  { }

				const std::vector<unsigned char> privkeyPrefix() const override;

				int32_t addrChecksumValue() const override;
			private:
				const std::map<std::string, std::string> &_resultMap;
		};
		std::unique_ptr<PrivateKeyHelper> _privHelper;
};

class KeysHelperWithFileMulti {
	public:
		KeysHelperWithFileMulti(const std::string Path);

        const IWalletAddrHelper& addrHelper() const;

        const IPrivateKeyHelper& privHelper() const;

	private:
		std::map<std::string, std::string> _resultMap;

		class WalletAddrHelper : public IWalletAddrHelper {
			public:
				WalletAddrHelper(const std::map<std::string, std::string> &result) :
					_resultMap(result)  { }

				const std::vector<unsigned char> pubkeyAddrPrefix() const override;
				const std::vector<unsigned char> scriptAddrPrefix() const override;

				int32_t addrChecksumValue() const override;
			private:
				const std::map<std::string, std::string> & _resultMap;

		};
		std::unique_ptr<WalletAddrHelper> _addrHelper;

		class PrivateKeyHelper : public IPrivateKeyHelper {
			public:
				PrivateKeyHelper(const std::map<std::string, std::string> &result) :
					_resultMap(result)  { }

				const std::vector<unsigned char> privkeyPrefix() const override;

				int32_t addrChecksumValue() const override;
			private:
				const std::map<std::string, std::string> &_resultMap;
		};
		std::unique_ptr<PrivateKeyHelper> _privHelper;
};


#endif // HS_KEYSHELPERWITHFILE_H
