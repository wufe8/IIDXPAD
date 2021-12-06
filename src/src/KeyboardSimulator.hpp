#include <memory>

enum KeyboardSimulatorLayout
{
	LAYOUT_YUANCON,
	LAYOUT_TASOLLER,
	LAYOUT_IIDX,
    LAYOUT_TEST
};

struct KeyboardSimulator
{

    struct Impl;
    std::unique_ptr<Impl> m_impl;

    KeyboardSimulator(KeyboardSimulatorLayout layout = LAYOUT_YUANCON);
    ~KeyboardSimulator();

    void send(uint64_t keys);
    void delay(int millis);
};
