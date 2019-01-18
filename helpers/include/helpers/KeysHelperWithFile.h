#ifndef HS_KEYSHELPERWITHFILE_H
#define HS_KEYSHELPERWITHFILE_H

#include <memory>
#include <keys/keyshelper.h>
#include <map>

#ifndef IS_SPACE
#define IS_SPACE(x) (x == ' ' || x == '\n' || x == '\r' || x == '\t' || x == '\v')
#endif

#define TRIM_SPACE " \t\n\v\r"

/**
 *
 * @brief 개인키와 지갑주소를 생성하기 위한 정보를 제공 한다.
 * @details 개인키와 지갑주소를 생성하기 위한 정보를 HDAC 네트워크의
 * 파라미터 설정 파일을 통해 가져 온다.
 * @author HDAC Technology Inc.
 * @date 2019-01-18
 * @version 0.0.1
 *
 */
class KeysHelperWithFileAll {
	public:
		KeysHelperWithFileAll(const std::string Path);

/**
 *
 * @brief 지갑주소 처리를 위한 정보 제공 인터페이스를 가져온다.
 * @details 주로 지갑주소 생성을 위해 내부적으로 사용 된다.
 * 
 * @return 지갑주소 처리를 위한 정보 제공 인터페이스
 *
 */
        const IWalletAddrHelper& addrHelper() const;

/**
 *
 * @brief 개인키 처리를 위한 정보 제공 인터페이스를 가져온다.
 * @details 주로 개인키 처리를 위해 내부적으로 사용 된다.
 * 
 * @return 개인키 처리를 위한 정보 제공 인터페이스
 *
 */
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
