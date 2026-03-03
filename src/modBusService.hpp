#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <string>
/* 2025 09 LX
读 数据			0x03
主机发送
器件地址(1byte) + 功能码(1byte) + 寄存器起始地址（2byte) + 读寄存器数量（2byte） + crc校验码（2byte）
例：0x01            0x03           0x00 0x32                 0x00 0x02               0xfe 0x23
从机响应
器件地址(1byte) + 功能码(1byte) + 读取的数据字节数(1byte)（每个寄存器两个字节所以要数量x2） + 寄存器数据（根据要读取的寄存器个数返回每个寄存器两个字节） + crc校验码（2byte）
例：0x01            0x03                 0x04                                                 0x00 0x3a        0x00 0x4d                               0xfa 0x43

写 单个数据		0x06
主机发送
器件地址(1byte) + 功能码(1byte) + 寄存器起始地址（2byte) + 修改寄存器的值（2byte） + crc校验码（2byte）
例：0x01            0x06           0x00 0x32                 0x00 0x02                 0xfe 0x23
从机响应 （与主机完全一致）
例：0x01            0x06           0x00 0x32                 0x00 0x02                 0xfe 0x23

写 多个数据		0x10
主机发送
器件地址(1byte) + 功能码(1byte) + 寄存器起始地址（2byte) + 写寄存器数量（2byte） +  数据段的总共长度（寄存器数量x2）(1byte)    数据（两个字节一组）  crc校验码（2byte）
例：0x01            0x10            0x00 0x32                 0x00 0x02                  0x04                                  0x00 0x23  0x00 0x45    0xa2 0x3f
从机响应 
器件地址(1byte) + 功能码(1byte) + 寄存器起始地址（2byte) + 写寄存器数量（2byte） + crc校验码（2byte）
例：0x01            0x10            0x00 0x32                 0x00 0x02                 0xfe 0x23
*/
// Modbus功能码枚举
enum class ModbusFunctionCode : uint8_t {
	ReadCoils = 0x01,
	ReadDiscreteInputs = 0x02,
	ReadHoldingRegisters = 0x03,
	ReadInputRegisters = 0x04,
	WriteSingleCoil = 0x05,
	WriteSingleRegister = 0x06,
	WriteMultipleCoils = 0x0F,
	WriteMultipleRegisters = 0x10
};

// Modbus异常码枚举
enum class ModbusExceptionCode : uint8_t {
	NoError = 0x00,
	IllegalFunction = 0x01,
	IllegalDataAddress = 0x02,
	IllegalDataValue = 0x03,
	SlaveDeviceFailure = 0x04,
	Acknowledge = 0x05,
	SlaveDeviceBusy = 0x06,
	NegativeAcknowledge = 0x07,
	MemoryParityError = 0x08
};
/*
int test() {
	try {
		// 创建读取请求
		auto readRequest = ModbusFrame::createRequest(
			0x01,
			ModbusFunctionCode::ReadHoldingRegisters,
			0x0000,
			0x000A
		);

		// 发送数据
		serialPort.write(readRequest->getRawData().data(),
			readRequest->getRawData().size());

		// 接收并解析响应
		std::vector<uint8_t> response = receiveData();
		auto responseFrame = ModbusFrame::parseResponse(response);

		if (responseFrame && responseFrame->isValid()) {
			if (responseFrame->isException()) {
				std::cout << "异常响应: " << responseFrame->toString() << std::endl;
			}
			else {
				std::cout << "成功响应: " << responseFrame->toString() << std::endl;
				auto data = responseFrame->getData();
				// 处理数据...
			}
		}

	}
	catch (const std::exception& e) {
		std::cerr << "错误: " << e.what() << std::endl;
	}

	return 0;
}

*/
class ModbusFrame {
private:
	uint8_t slaveAddress_;
	ModbusFunctionCode functionCode_;
	ModbusExceptionCode exceptionCode_;
	uint16_t startAddress_;
	uint16_t quantity_;
	std::vector<uint8_t> data_;
	std::vector<uint8_t> rawData_;
	bool isValid_;
#if 0
	static inline const uint16_t crcTable[] = {
		0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
		0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
		0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
		0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
		0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
		0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
		0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
		0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
		0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
		0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
		0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
		0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
		0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
		0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
		0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
		0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
		0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
		0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
		0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
		0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
		0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
		0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
		0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
		0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
		0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
		0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
		0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
		0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
		0x4200, 0x82C1, 0x8381, 0x4340, 0x8101, 0x41C0, 0x4080, 0x8041
};
#else
	static inline const uint16_t crcTable[] = {
	0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
	0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
	0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
	0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
	0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
	0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
	0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
	0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
	0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
	0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
	0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
	0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
	0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
	0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
	0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
	0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
	0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
	0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
	0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
	0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
	0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
	0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
	0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
	0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
	0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
	0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
	0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
	0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
	0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
	0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
	0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
	0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040,
	};
#endif // 0 
private:
	// 私有构造函数，防止无效构造
	ModbusFrame() : isValid_(false) {}
	// 私有构造函数
	ModbusFrame(
		uint8_t slaveAddr, 
		ModbusFunctionCode funcCode,
		uint16_t startAddr, 
		uint16_t qty,
		const std::vector<uint8_t>& dataBytes)
		: slaveAddress_(slaveAddr), 
		functionCode_(funcCode),
		exceptionCode_(ModbusExceptionCode::NoError),
		startAddress_(startAddr), 
		quantity_(qty), 
		data_(dataBytes),
		isValid_(true) 
	{
		buildRequestFrame();
	}
public:
	// 工厂方法：创建请求帧
	static std::unique_ptr<ModbusFrame> createRequest(
		uint8_t slaveAddr,
		ModbusFunctionCode funcCode,
		uint16_t startAddr = 0,
		uint16_t qty = 0,
		const std::vector<uint8_t>& dataBytes = {}) {
		// 使用 std::make_unique 并调用私有构造函数
		return std::unique_ptr<ModbusFrame>(new ModbusFrame(slaveAddr, funcCode, startAddr, qty, dataBytes));
	}

	// 工厂方法：从原始数据解析响应帧
	static std::unique_ptr<ModbusFrame> parseResponse(
		const std::vector<uint8_t>& receivedData) {

		auto frame = std::unique_ptr<ModbusFrame>(new ModbusFrame());
		if (frame->parseResponseFrame(receivedData)) {
			frame->isValid_ = true;
			return frame;
		}
		return nullptr; // 解析失败返回空指针
	}

	// 获取原始字节数据（用于发送）
	const std::vector<uint8_t>& getRawData() const {
		if (!isValid_) throw std::runtime_error("帧无效");
		return rawData_;
	}

	// 获取解析的数据
	const std::vector<uint8_t>& getData() const {
		if (!isValid_) throw std::runtime_error("帧无效");
		return data_;
	}

	// 获取从站地址
	uint8_t getSlaveAddress() const {
		if (!isValid_) throw std::runtime_error("帧无效");
		return slaveAddress_;
	}

	// 获取功能码
	ModbusFunctionCode getFunctionCode() const {
		if (!isValid_) throw std::runtime_error("帧无效");
		return functionCode_;
	}

	// 获取异常码
	ModbusExceptionCode getExceptionCode() const {
		if (!isValid_) throw std::runtime_error("帧无效");
		return exceptionCode_;
	}

	// 检查是否有效
	bool isValid() const { return isValid_; }

	// 检查是否是异常响应
	bool isException() const {
		return isValid_ && exceptionCode_ != ModbusExceptionCode::NoError;
	}

	// 转换为字符串表示（调试用）
	std::string toString() const {
		if (!isValid_) return "Invalid Frame";
		
		std::string result = "Slave: " + std::to_string(slaveAddress_) +
			", Func: " + std::to_string(static_cast<int>(functionCode_));

		if (isException()) {
			result += " [Exception: " + std::to_string(static_cast<int>(exceptionCode_)) + "]";
		}

		return result;
	}

private:
	// 构建请求帧
	void buildRequestFrame() {
		rawData_.clear();

		rawData_.push_back(slaveAddress_);
		rawData_.push_back(static_cast<uint8_t>(functionCode_));

		// 根据功能码构建不同的帧格式
		switch (functionCode_) {
		case ModbusFunctionCode::ReadCoils:
		case ModbusFunctionCode::ReadDiscreteInputs:
		case ModbusFunctionCode::ReadHoldingRegisters:
		case ModbusFunctionCode::ReadInputRegisters:
			// 读操作：地址 + 数量
			appendUint16(startAddress_);
			appendUint16(quantity_);
			break;

		case ModbusFunctionCode::WriteSingleCoil:
		case ModbusFunctionCode::WriteSingleRegister:
			// 写单个：地址 + 数据值
			appendUint16(startAddress_);
			appendUint16(quantity_); // 这里quantity代表要写入的值
			break;

		case ModbusFunctionCode::WriteMultipleCoils:
		case ModbusFunctionCode::WriteMultipleRegisters:
			// 写多个：地址 + 数量 + 字节数 + 数据
			appendUint16(startAddress_);
			appendUint16(quantity_);
			if (!data_.empty()) {
				rawData_.push_back(static_cast<uint8_t>(data_.size()));
				rawData_.insert(rawData_.end(), data_.begin(), data_.end());
			}
			break;

		default:
			throw std::runtime_error("不支持的功能码");
		}

		// 添加CRC
		appendCRC(rawData_);
	}

	// 辅助函数：添加16位数据（低字节在前）
	void appendUint16(uint16_t value) {
		rawData_.push_back(static_cast<uint8_t>(value >> 8));   // 高字节
		rawData_.push_back(static_cast<uint8_t>(value & 0xFF)); // 低字节
	}
	// 解析响应帧
	bool parseResponseFrame(const std::vector<uint8_t>& frame) {
		if (frame.size() < 5) return false;
		if (!verifyCRC(frame)) return false;

		rawData_ = frame;
		slaveAddress_ = frame[0];

		// 检查异常响应  异常时会把最高位中设置为1
		if (frame[1] & 0x80) {
			functionCode_ = static_cast<ModbusFunctionCode>(frame[1] & 0x7F);
			exceptionCode_ = static_cast<ModbusExceptionCode>(frame[2]);
			return true;
		}

		functionCode_ = static_cast<ModbusFunctionCode>(frame[1]);
		exceptionCode_ = ModbusExceptionCode::NoError;

		// 解析响应数据
		return parseResponseData(frame);
	}

	bool parseResponseData(const std::vector<uint8_t>& frame) {
		try {
			switch (functionCode_) {
			case ModbusFunctionCode::ReadHoldingRegisters:
			case ModbusFunctionCode::ReadInputRegisters:
			case ModbusFunctionCode::ReadCoils:
			case ModbusFunctionCode::ReadDiscreteInputs: {
				uint8_t byteCount = frame[2];
				if (frame.size() < 3 + byteCount + 2) 
					return false; // +2 for CRC
				data_.assign(frame.begin() + 3, frame.begin() + 3 + byteCount);
				break;
			}
			case ModbusFunctionCode::WriteSingleRegister:
			case ModbusFunctionCode::WriteSingleCoil: {
				startAddress_ = (frame[2] << 8) | frame[3];
				quantity_ = (frame[4] << 8) | frame[5];
				break;
			}
			case ModbusFunctionCode::WriteMultipleRegisters:
			case ModbusFunctionCode::WriteMultipleCoils: {
				startAddress_ = (frame[2] << 8) | frame[3];
				quantity_ = (frame[4] << 8) | frame[5];
				break;
			}
			default:
				break;
			}
			return true;
		}
		catch (...) {
			return false;
		}
	}

	// CRC相关静态方法
	static uint16_t calculateCRC(const uint8_t* data, size_t length) {
		uint16_t crc = 0xFFFF;
		for (size_t i = 0; i < length; ++i) {
			uint8_t index = (crc ^ data[i]) & 0xFF;
			crc = (crc >> 8) ^ crcTable[index];
		}
		return crc;
	}

	static void appendCRC(std::vector<uint8_t>& data) {
		uint16_t crc = calculateCRC(data.data(), data.size());
		data.push_back(crc & 0xFF);
		data.push_back((crc >> 8) & 0xFF);
	}

	static bool verifyCRC(const std::vector<uint8_t>& data) {
		return data.size() >= 2 && calculateCRC(data.data(), data.size()) == 0;
	}
};
