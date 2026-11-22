/*
 * Host-side regression tests for the Morse library.
 *
 * Builds Morse.cpp against a stub Arduino.h and drives update() one tick
 * (1 ms) at a time, checking the resulting pin timing. Run with `make` in
 * this directory.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

#include "Arduino.h"

// Allow tests to inspect and set internal state (e.g. the tick counter)
#define private public
#include "Morse.h"
#undef private

namespace sim
{
uint32_t now;
uint8_t pin_value[256];
uint8_t pin_mode[256];
std::vector<PinEvent> events;
}

static int failures = 0;
static const char *current_test = "";

#define CHECK(cond) do { if (!(cond)) { \
  printf("FAIL [%s] %s:%d: %s\n", current_test, __FILE__, __LINE__, #cond); \
  failures++; } } while (0)

#define CHECK_EQ(a, b) do { long long va_ = (long long)(a), vb_ = (long long)(b); \
  if (va_ != vb_) { \
  printf("FAIL [%s] %s:%d: %s == %s (%lld vs %lld)\n", current_test, __FILE__, \
    __LINE__, #a, #b, va_, vb_); failures++; } } while (0)

static const uint8_t PIN = 9;
static const float WPM = 20;          // 60 ms dit
static const uint32_t DIT = 60;

static void begin(const char *name)
{
  current_test = name;
  sim::now = 0;
  memset(sim::pin_value, 0, sizeof(sim::pin_value));
  memset(sim::pin_mode, 0, sizeof(sim::pin_mode));
  sim::events.clear();
}

static void run(Morse &m, uint32_t ticks)
{
  for (uint32_t i = 0; i < ticks; i++)
  {
    sim::now++;
    m.update();
  }
}

static bool runUntilDone(Morse &m, uint32_t max_ticks = 200000)
{
  for (uint32_t i = 0; i < max_ticks && m.busy; i++)
  {
    sim::now++;
    m.update();
  }
  return !m.busy;
}

// Key-down intervals on a pin as (start tick, end tick) pairs
static std::vector<std::pair<uint32_t, uint32_t>> keyIntervals(uint8_t pin)
{
  std::vector<std::pair<uint32_t, uint32_t>> out;
  uint32_t start = 0;
  bool down = false;

  for (const PinEvent &e : sim::events)
  {
    if (e.pin != pin) continue;
    if (e.value == HIGH && !down) { start = e.tick; down = true; }
    else if (e.value == LOW && down) { out.push_back({start, e.tick}); down = false; }
  }
  return out;
}

static size_t eventCount(uint8_t pin)
{
  size_t n = 0;
  for (const PinEvent &e : sim::events) if (e.pin == pin) n++;
  return n;
}

// Send a message and return its key-down intervals, shifted to start at 0
static std::vector<std::pair<uint32_t, uint32_t>> sendAndCapture(const char *msg)
{
  Morse m(PIN, WPM);
  m.send(msg);
  runUntilDone(m);
  std::vector<std::pair<uint32_t, uint32_t>> k = keyIntervals(PIN);
  if (!k.empty())
  {
    uint32_t base = k[0].first;
    for (auto &iv : k) { iv.first -= base; iv.second -= base; }
  }
  return k;
}

static void testElementTiming()
{
  begin("element timing");
  auto k = sendAndCapture("IT");    // .. -
  CHECK_EQ(k.size(), 3);
  if (k.size() != 3) return;
  CHECK_EQ(k[0].second - k[0].first, DIT);          // dit
  CHECK_EQ(k[1].first - k[0].second, DIT);          // element gap
  CHECK_EQ(k[1].second - k[1].first, DIT);
  CHECK_EQ(k[2].first - k[1].second, 3 * DIT);      // character gap
  CHECK_EQ(k[2].second - k[2].first, 3 * DIT);      // dah
}

static void testWordSpacing()
{
  begin("word spacing");
  auto k = sendAndCapture("E E");
  CHECK_EQ(k.size(), 2);
  if (k.size() != 2) return;
  CHECK_EQ(k[1].first - k[0].second, 7 * DIT);
}

static void testParisStandard()
{
  begin("PARIS standard");
  // "PARIS " is 50 dits by definition of WPM
  auto k = sendAndCapture("PARIS PARIS");
  CHECK_EQ(k.size(), 28);
  if (k.size() != 28) return;
  CHECK_EQ(k[14].first - k[0].first, 50 * DIT);
}

static void testEndOfMessage()
{
  begin("end of message");
  Morse m(PIN, WPM);
  m.send("E");
  run(m, 1);
  CHECK(m.busy);
  CHECK(m.tx);
  CHECK(runUntilDone(m));
  auto k = keyIntervals(PIN);
  CHECK_EQ(k.size(), 1);
  // Busy until a full word space after the last element
  CHECK_EQ(sim::now - k[0].second, 7 * DIT);
  CHECK(!m.tx);

  // Parks quietly in IDLE afterwards
  size_t events = sim::events.size();
  run(m, 10000);
  CHECK_EQ(sim::events.size(), events);
  CHECK(m.cur_state == Morse::State::IDLE);
  CHECK(!m.busy);
}

static void testEmptyMessage()
{
  begin("empty message");
  Morse m(PIN, WPM);
  m.send("");
  run(m, 5000);
  CHECK_EQ(eventCount(PIN), 0);
  CHECK(!m.busy);
}

static void testUnsupportedCharactersSkipped()
{
  begin("unsupported characters skipped");
  auto expected = sendAndCapture("EEEEEE");
  begin("unsupported characters skipped");
  auto actual = sendAndCapture("E\tE~E\x80" "E\nE{E");
  CHECK(expected == actual);

  begin("unsupported-only message");
  Morse m(PIN, WPM);
  m.send("\t\r\n~");
  run(m, 5000);
  CHECK_EQ(eventCount(PIN), 0);
  CHECK(!m.busy);
}

static void testLowercase()
{
  begin("lowercase");
  auto upper = sendAndCapture("HELLO WORLD 73");
  begin("lowercase");
  auto lower = sendAndCapture("hello world 73");
  CHECK(!upper.empty());
  CHECK(upper == lower);
}

static void testPunctuation()
{
  begin("punctuation");
  auto k = sendAndCapture("/");     // -..-.
  CHECK_EQ(k.size(), 5);
  if (k.size() != 5) return;
  CHECK_EQ(k[0].second - k[0].first, 3 * DIT);
  CHECK_EQ(k[1].second - k[1].first, DIT);
  CHECK_EQ(k[2].second - k[2].first, DIT);
  CHECK_EQ(k[3].second - k[3].first, 3 * DIT);
  CHECK_EQ(k[4].second - k[4].first, DIT);
}

static void testLongMessageTruncated()
{
  begin("long message truncated");
  std::string msg(150, 'E');
  Morse m(PIN, WPM);
  m.send(msg.c_str());
  CHECK_EQ(strlen(m.msg_buffer), Morse::TX_BUFFER_SIZE);
  CHECK(runUntilDone(m, 1000000));
  CHECK_EQ(keyIntervals(PIN).size(), Morse::TX_BUFFER_SIZE);
}

static void testCurChar()
{
  begin("cur_char");
  Morse m(PIN, WPM);
  m.send("AB C");
  run(m, 1);
  CHECK_EQ(m.cur_char, 0);
  // A (.-) is dit, gap, dah, gap = 6 dits; still on A just before that
  run(m, 6 * DIT - 1);
  CHECK_EQ(m.cur_char, 0);
  // ...then into the character gap before B
  run(m, 1);
  CHECK_EQ(m.cur_char, 1);
  runUntilDone(m);
  CHECK_EQ(m.cur_char, 0);
}

static void testReset()
{
  begin("reset mid-element");
  Morse m(PIN, WPM);
  m.send("TTT");
  run(m, 30);
  CHECK(sim::pin_value[PIN] == HIGH);
  m.reset();
  CHECK(sim::pin_value[PIN] == LOW);
  CHECK(!m.busy);
  CHECK(!m.tx);
  size_t events = sim::events.size();
  run(m, 5000);
  CHECK_EQ(sim::events.size(), events);

  begin("reset while tx disabled");
  Morse n(PIN, WPM);
  n.send("TTT");
  run(n, 30);
  n.tx_enable = false;
  n.reset();
  CHECK(sim::pin_value[PIN] == LOW);
}

static void testSendWhileBusy()
{
  begin("send while busy");
  Morse m(PIN, WPM);
  m.send("TTTTTTTTTT");
  run(m, 30);
  m.send("E");
  CHECK(sim::pin_value[PIN] == LOW);
  CHECK(runUntilDone(m));
  auto k = keyIntervals(PIN);
  CHECK_EQ(k.size(), 2);              // The cut-off T, then the E
  if (k.size() == 2) CHECK_EQ(k[1].second - k[1].first, DIT);
}

static void testTxEnablePause()
{
  begin("tx_enable pause");
  Morse m(PIN, WPM);
  m.send("EE");
  run(m, DIT + 10);                   // In the gap after the first E
  m.tx_enable = false;
  run(m, 5000);
  CHECK(m.busy);
  CHECK_EQ(keyIntervals(PIN).size(), 1);
  CHECK(sim::pin_value[PIN] == LOW);
  m.tx_enable = true;
  CHECK(runUntilDone(m));
  CHECK_EQ(keyIntervals(PIN).size(), 2);
}

static void testPreamble()
{
  begin("preamble");
  Morse m(PIN, WPM);
  m.preamble_enable = true;
  m.send("E");
  CHECK(runUntilDone(m));
  auto k = keyIntervals(PIN);
  CHECK_EQ(k.size(), 1);
  if (k.size() == 1) CHECK_EQ(k[0].first, 1 + 7 * DIT);
  CHECK(!m.preamble_enable);          // One-shot
}

static void testDfcw()
{
  begin("DFCW");
  Morse m(PIN, WPM);
  m.dfcw_mode = true;
  m.send("ET");
  int tx_edges = 0;
  bool last_tx = false;
  for (int i = 0; i < 100000 && m.busy; i++)
  {
    sim::now++;
    m.update();
    if (m.tx != last_tx) { tx_edges++; last_tx = m.tx; }
  }
  CHECK(!m.busy);
  // Carrier on for the whole message, off at the end
  CHECK_EQ(eventCount(PIN), 2);
  CHECK(sim::pin_value[PIN] == LOW);
  CHECK_EQ(tx_edges, 4);
}

static void testPinZeroUnused()
{
  begin("pin 0 unused");
  Morse m(0, WPM);
  m.send("SOS");
  int tx_edges = 0;
  bool last_tx = false;
  for (int i = 0; i < 100000 && m.busy; i++)
  {
    sim::now++;
    m.update();
    if (m.tx != last_tx) { tx_edges++; last_tx = m.tx; }
  }
  m.reset();
  CHECK_EQ(eventCount(0), 0);
  CHECK_EQ(sim::pin_mode[0], INPUT);
  CHECK_EQ(tx_edges, 18);
}

static void testSeparateLedPin()
{
  begin("separate LED pin");
  Morse m(PIN, WPM);
  m.led_pin = 7;
  m.send("A");
  CHECK_EQ(sim::pin_mode[7], OUTPUT);
  CHECK(runUntilDone(m));
  CHECK(keyIntervals(7) == keyIntervals(PIN));
  CHECK_EQ(keyIntervals(7).size(), 2);
}

static void testSetWpmLimits()
{
  begin("setWPM limits");
  Morse m(PIN, WPM);
  m.setWPM(0);
  CHECK(m.wpm == Morse::MIN_WPM);
  CHECK(m.dit_length > 0);
  m.setWPM(-5);
  CHECK(m.wpm == Morse::MIN_WPM);
  m.setWPM(NAN);
  CHECK(m.wpm == Morse::MIN_WPM);
  m.setWPM(100000);
  CHECK(m.wpm == Morse::MAX_WPM);
  CHECK_EQ(m.dit_length, 1);
  m.setWPM(0.2f);                     // QRSS: 6 second dit
  CHECK_EQ(m.dit_length, 6000);
  m.setWPM(25);
  CHECK_EQ(m.dit_length, 48);
}

static void testTimerWrap()
{
  // Start so that a state ends exactly at 0xFFFFFFFF, which previously hung
  for (uint32_t offset = 0; offset < 200; offset += 7)
  {
    begin("timer wrap");
    Morse m(PIN, WPM);
    m.cur_timer = 0xFFFFFFFFUL - DIT - 1 - offset;
    m.send("EEE");
    CHECK(runUntilDone(m));
    auto k = keyIntervals(PIN);
    CHECK_EQ(k.size(), 3);
    for (auto &iv : k) CHECK_EQ(iv.second - iv.first, DIT);
  }
}

int main()
{
  testElementTiming();
  testWordSpacing();
  testParisStandard();
  testEndOfMessage();
  testEmptyMessage();
  testUnsupportedCharactersSkipped();
  testLowercase();
  testPunctuation();
  testLongMessageTruncated();
  testCurChar();
  testReset();
  testSendWhileBusy();
  testTxEnablePause();
  testPreamble();
  testDfcw();
  testPinZeroUnused();
  testSeparateLedPin();
  testSetWpmLimits();
  testTimerWrap();

  if (failures)
  {
    printf("%d check(s) failed\n", failures);
    return 1;
  }
  printf("All tests passed\n");
  return 0;
}
