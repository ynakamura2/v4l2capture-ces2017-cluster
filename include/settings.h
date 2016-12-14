#ifndef SETTINGS_H_
#define SETTINGS_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#include <string>

class Settings {
 public:
	Settings() {}
	~Settings() {}

	int GetCaptureWidth()       const { return width_;  }
	int GetCaptureHeight()      const { return height_; }
	std::string GetDeviceName() const { return device_name_; }

	void SetWidth(int width)   { width_  = width;  }
	void SetHeight(int height) { height_ = height; }
	void SetDeviceName(std::string device_name) { device_name_ = device_name; }

 private:
	int width_;
	int height_;

	std::string device_name_;

 private:
	DISALLOW_COPY_AND_ASSIGN(Settings);
};
#endif // SETTINGS_H_
