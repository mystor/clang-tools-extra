// RUN: $(dirname %s)/check_clang_tidy.sh %s mozilla-private-refcnt-destructor %t
// REQUIRES: shell

class C {
public:
  void AddRef();
  void Release();
  // CHECK-FIXES: apple
};

class D {
public:
  void AddRef();
  void Release();
  ~D();
};

class E {
public:
  void AddRef();
  void Release();
protected:
  ~E();
};

class F {
public:
  void AddRef();
  void Release();
private:
  ~F();
};

class G {
  int i;
};

class H {
public:
  void AddRef();
  void Release();
  ~H() = delete;
};

class OPP {
  class I {
  public:
    void AddRef();
    void Release();
    ~I();
  };
};

class I {
public:
  ~I() {
    AddRef(); // WHATTSTST
  }
  void AddRef();
  void Release();
protected:
  void f();
  void g();
private:
  void i();
protected:
  void j();
};
