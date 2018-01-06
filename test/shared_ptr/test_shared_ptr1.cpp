#include <iostream>  
#include <memory> // shared_ptr  
#include <thread>  
#include <chrono>  
#include <mutex>  
#include <queue>  
//////////////////////////////////////////////////////  
// reference: http://en.cppreference.com/w/cpp/memory/shared_ptr  
struct Base  
{  
    Base() { std::cout << "  Base::Base()\n"; }  
    // Note: non-virtual destructor is OK here  
    ~Base() { std::cout << "  Base::~Base()\n"; }  
};  
  
struct Derived : public Base  
{  
    Derived() { std::cout << "  Derived::Derived()\n"; }  
	Derived(std::string name) { name_ = name; std::cout << name_ << ",  Derived::Derived()\n"; }  
    ~Derived() { std::cout << name_<<",  Derived::~Derived()\n"; }  
	std::string name_;
};  
  
void thr(std::shared_ptr<Base> p)  
{  
    std::this_thread::sleep_for(std::chrono::seconds(1));  
    std::shared_ptr<Base> lp = p; // thread-safe, even though the shared use_count is incremented  
    {  
        static std::mutex io_mutex;  
        std::lock_guard<std::mutex> lk(io_mutex);  
        std::cout << "local pointer in a thread:\n"  
            << "  lp.get() = " << lp.get()  
            << ", lp.use_count() = " << lp.use_count() << '\n';  
    }  
}  
  
int test_shared_ptr1()  
{  
    std::shared_ptr<Base> p = std::make_shared<Derived>();  
  
    std::cout << "Created a shared Derived (as a pointer to Base)\n"  
        << "  p.get() = " << p.get()  
        << ", p.use_count() = " << p.use_count() << '\n' << '\n';  
    std::thread t1(thr, p), t2(thr, p), t3(thr, p);  
    //p.reset(); // release ownership from main  
    std::cout << "Shared ownership between 3 threads and released\n"  
        << "ownership from main:\n"  
        << "  p.get() = " << p.get()  
        << ", p.use_count() = " << p.use_count() << '\n' << '\n';  
    t1.join(); t2.join(); t3.join();  
    std::cout << "All threads completed, the last one deleted Derived\n";  
  
    return 0;  
} 

std::shared_ptr<Base> test_shared_ptr1_ext()  
{  
    std::shared_ptr<Base> p  = std::make_shared<Derived>();  
  
    std::cout << "Created a shared Derived (as a pointer to Base)\n"  
        << "  p.get() = " << p.get()  
        << ", p.use_count() = " << p.use_count() << '\n' << '\n';  
    std::thread t1(thr, p), t2(thr, p), t3(thr, p);  
  //  p.reset(); // release ownership from main  
    std::cout << "Shared ownership between 3 threads and released\n"  
        << "ownership from main:\n"  
        << "  p.get() = " << p.get()  
        << ", p.use_count() = " << p.use_count() << '\n' << '\n';  
    t1.join(); t2.join(); t3.join();  
    std::cout << "All threads completed, the last one deleted Derived\n";  
  
    return p;  
} 

class SmartPtr
{
public:
	SmartPtr()
	{
		std::string  str = "smart";
		 std::shared_ptr<Base> p  = std::make_shared<Derived>(str); 		// 资源在析构的时候会被释放
		 p2 = new  Derived();					// 资源没有被释放
		 queue_.push(p);
		 queue_.push(p);
		 queue_.push(std::make_shared<Derived>("lqf"));
	}
	~SmartPtr(){}
private:	
	//std::shared_ptr<Base> p;
	Base *p2;
	std::queue<std::shared_ptr<Base>> queue_;		//  当类被销毁时,对应的成员也会被销毁，仅限于智能指针
};

///////////////////////////////////////////////  
// reference: http://www.cplusplus.com/reference/memory/shared_ptr/shared_ptr/  
int test_shared_ptr2()  
{  
    struct C { int* data; };  
  
    // shared_ptr constructor example  
    std::shared_ptr<int> p1;  
    std::shared_ptr<int> p2(nullptr);  
    std::shared_ptr<int> p3(new int);  
    std::shared_ptr<int> p4(new int, std::default_delete<int>());  
    std::shared_ptr<int> p5(new int, [](int* p){delete p; }, std::allocator<int>());  
    std::shared_ptr<int> p6(p5);  
    std::shared_ptr<int> p7(std::move(p6));  
    std::shared_ptr<int> p8(std::unique_ptr<int>(new int));  
    std::shared_ptr<C> obj(new C);  
    std::shared_ptr<int> p9(obj, obj->data);  
  
    std::cout << "use_count:\n";  
    std::cout << "p1: " << p1.use_count() << '\n'; // 0  
    std::cout << "p2: " << p2.use_count() << '\n'; // 0  
    std::cout << "p3: " << p3.use_count() << '\n'; // 1  
    std::cout << "p4: " << p4.use_count() << '\n'; // 1  
    std::cout << "p5: " << p5.use_count() << '\n'; // 2  
    std::cout << "p6: " << p6.use_count() << '\n'; // 0  
    std::cout << "p7: " << p7.use_count() << '\n'; // 2  
    std::cout << "p8: " << p8.use_count() << '\n'; // 1  
    std::cout << "p9: " << p9.use_count() << '\n'; // 2  
  
    return 0;  
}  
  
//////////////////////////////////////////////////////////  
// reference: https://oopscenities.net/2013/10/06/smart-pointers-part-4-shared_ptr/  
class Integer  
{  
    int n;  
public:  
    Integer(int n) : n(n) { }  
    ~Integer() { printf("Deleting %d\n", n); }  
    int get() const { return n; }  
};  
  
int test_shared_ptr3()  
{  
    auto a = std::make_shared<Integer>(10);  
    auto b = std::make_shared<Integer>(20);  
    auto c = a;  
    auto d = std::make_shared<Integer>(30);  
    auto e = b;  
    a = d;  
    b = std::make_shared<Integer>(40);  
    auto f = c;  
    b = f;  
  
    printf("%d\n", a->get());  
    printf("%d\n", b->get());  
    printf("%d\n", c->get());  
    printf("%d\n", d->get());  
    printf("%d\n", e->get());  
    printf("%d\n", f->get());  
  
    return 0;  
}  
  
//////////////////////////////////////////////  
// reference: http://www.linux-magazin.de/Ausgaben/2013/04/C-11  
struct MyInt{  
    MyInt(int v) :val(v){  
        std::cout << "  Hello: " << val << std::endl;  
    }  
    ~MyInt(){  
        std::cout << "  Good Bye: " << val << std::endl;  
    }  
    int val;  
};  
  
int test_shared_ptr4()  
{  
    std::shared_ptr<MyInt> sharPtr(new MyInt(1998));  
    std::cout << "    My value: " << sharPtr->val << std::endl;  
    std::cout << "sharedPtr.use_count(): " << sharPtr.use_count() << std::endl;  
  
    {  
        std::shared_ptr<MyInt> locSharPtr(sharPtr);  
        std::cout << "locSharPtr.use_count(): " << locSharPtr.use_count() << std::endl;  
    }  
    std::cout << "sharPtr.use_count(): " << sharPtr.use_count() << std::endl;  
  
    std::shared_ptr<MyInt> globSharPtr = sharPtr;  
    std::cout << "sharPtr.use_count(): " << sharPtr.use_count() << std::endl;  
    globSharPtr.reset();  
    std::cout << "sharPtr.use_count(): " << sharPtr.use_count() << std::endl;  
  
    sharPtr = std::shared_ptr<MyInt>(new MyInt(2011));  
  
    return 0;  
}  
  
////////////////////////////////////////////////////////  
// reference: http://www.linux-magazin.de/Ausgaben/2013/04/C-11  
template <typename T>  
struct Deleter{  
    void operator()(T *ptr){  
        ++Deleter::count;  
        delete ptr;  
    }  
    static int count;  
};  
  
template <typename T>  
int Deleter<T>::count = 0;  
  
typedef Deleter<int> IntDeleter;  
typedef Deleter<double> DoubleDeleter;  
typedef Deleter<MyInt> MyIntDeleter;  
  
int test_shared_ptr5()  
{  
    {  
        std::shared_ptr<int> sharedPtr1(new int(1998), IntDeleter());  
        std::shared_ptr<int> sharedPtr2(new int(2011), IntDeleter());  
        std::shared_ptr<double> sharedPtr3(new double(3.17), DoubleDeleter());  
        std::shared_ptr<MyInt> sharedPtr4(new MyInt(2017), MyIntDeleter());  
    }  
  
    std::cout << "Deleted " << IntDeleter().count << " int values." << std::endl;  
    std::cout << "Deleted " << DoubleDeleter().count << " double value." << std::endl;  
    std::cout << "Deleted " << MyIntDeleter().count << " MyInt value." << std::endl;  
  
    return 0;  
}  

// g++ -std=c++11 -o test_shared_ptr1 test_shared_ptr1.cpp -lpthread
int main(void)
{
	int ret;
	//ret = test_shared_ptr1();
	/*
	std::cout << std::endl << std::endl;
	std::shared_ptr<Base> p1 = test_shared_ptr2();
	std::cout << "local pointer in a thread:\n"  
            << "  lp.get() = " << p1.get()  
            << ", lp.use_count() = " << p1.use_count() << '\n';  
	*/
	//ret = test_shared_ptr2();

	SmartPtr *smartPtr = new SmartPtr();
	delete smartPtr;
	std::cout << "exit" << std::endl;
	return 0;
}

