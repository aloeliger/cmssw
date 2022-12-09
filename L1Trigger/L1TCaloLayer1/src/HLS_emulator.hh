//The HLS4ML team has provided this interface to compiled model .so objects for the purpose of emulation
#ifndef HLS4ML_EMULATOR_H_
#define HLS4ML_EMULATOR_H_

#include <iostream>
#include <string>
#include <any>
#include <dlfcn.h>

class HLS4MLModel {
public:
  virtual void prepare_input(std::any input) = 0;
  virtual void predict() = 0;
  virtual void read_result(std::any result) = 0;
  //TODO: A virtual destructor should be included here to prevent SCRAM compiler warnings
  //However, it would be necessary to ensure that both the general and CICADA specific implementations
  //in the compiled model also matched up to this to not potentially cause side effects.
};

typedef HLS4MLModel* create_model_cls();
typedef void destroy_model_cls(HLS4MLModel*);

class ModelLoader {
private:
  std::string _model_name;
  void* _model_lib;
  HLS4MLModel* _model = nullptr;

public:
  ModelLoader(std::string model_name) {
    _model_name = model_name;
    _model_name.append(".so");
  }

  ~ModelLoader() { dlclose(_model_lib); }

  HLS4MLModel* load_model() {
    _model_lib = dlopen(_model_name.c_str(), RTLD_LAZY);
    if (!_model_lib) {
      std::cerr << "Cannot load library: " << dlerror() << std::endl;
      return nullptr;
    }

    create_model_cls* create_model = (create_model_cls*)dlsym(_model_lib, "create_model");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
      std::cerr << "Cannot load symbol 'create_model': " << dlsym_error << std::endl;
      return nullptr;
    }

    _model = create_model();

    return _model;
  }

  void destroy_model() {
    destroy_model_cls* destroy = (destroy_model_cls*)dlsym(_model_lib, "destroy_model");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
      std::cerr << "Cannot load symbol destroy_model: " << dlsym_error << std::endl;
    }
    if (_model != nullptr) {
      destroy(_model);
    }
  }
};

#endif
