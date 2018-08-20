#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <cmath>
#include <chrono>
#include <thread>

typedef std::pair<uint32_t, uint16_t> _adcreading;
std::list<_adcreading> adcreadings;
std::list<_adcreading>::iterator iter_readings;

uint16_t inputPinReader()
{
  return iter_readings != adcreadings.end() ? (*(++iter_readings)).second : UINT16_MAX;
}

_adcreading previous_reading(UINT32_MAX, UINT16_MAX);
uint16_t inputPinReaderDelayed()
{
  if (iter_readings == adcreadings.end())
  {
    return UINT16_MAX;
  }
  _adcreading reading = *(++iter_readings);
  if (previous_reading.first != UINT32_MAX)
  {
    uint32_t delay = reading.first - previous_reading.first;
    if (delay < 5000)
    {
      // std::cout << "delay " << reading.first << " " << previous_reading.first << " " << delay << std::endl;
      std::this_thread::sleep_for(std::chrono::microseconds(delay));
    }
    else
    {
      return UINT16_MAX;
    }
  }
  previous_reading = reading;
  return reading.second;
}

#define ADC_BITS 11
#define ADC_COUNTS (1 << ADC_BITS)

uint32_t calcRMS(uint16_t offsetI, float adjustment, uint16_t _crossings)
{
  int32_t sumI = 0.0;
  uint16_t sampleI;
  int32_t filteredI;
  uint16_t samples_taken = 0;
  bool down = false;

  sampleI = inputPinReader();
  if (sampleI > offsetI)
  {
    while (sampleI >= offsetI)
    {
      sampleI = inputPinReader();
      down = true;
    }
  }
  else
  {
    while (sampleI <= offsetI)
    {
      sampleI = inputPinReader();
      down = false;
    }
  }
  uint16_t crossings = 0;

  while (true)
  {
    sampleI = inputPinReader();
    if (sampleI == UINT16_MAX)
    {
      return UINT16_MAX;
    }
    if (down)
    {
      if (sampleI >= offsetI)
      {
        down = false;
        crossings++;
      }
    }
    else
    {
      if (sampleI <= offsetI)
      {
        down = true;
        crossings++;
      }
    }
    if (crossings == _crossings)
    {
      break;
    }
    samples_taken++;
    filteredI = sampleI - offsetI;
    sumI += (filteredI * filteredI);

    // std::cout << "D crossings " << crossings << " direction " << (down ? " DOWN " : " UP ") << " sampleI " << sampleI << " offsetI " << offsetI << " filteredI " << filteredI << " sumI " << sumI << std::endl;
  }

  uint32_t Irms = adjustment * std::sqrt(sumI / samples_taken);
  // std::cout << Irms << " " << sumI << " " << samples_taken  << std::endl;
  return Irms;
}

void testIntegerIRMS()
{
  uint32_t sum = 0;
  uint16_t ctr = 0;
  iter_readings = adcreadings.begin();
  auto start = std::chrono::high_resolution_clock::now();
  while (iter_readings != adcreadings.end())
  {
    // uint32_t irms = calcIrmsI(50);
    uint32_t irms = calcRMS(3300 / 4, 6, 10);
    if (irms == UINT16_MAX)
    {
      break;
    }
    sum += irms;
    ctr++;
    std::cout << "IRMS: " << (irms / 100.0) << std::endl;
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::micro> diff = end - start;
  std::cout << "Integer Took Time " << diff.count() << " IRMS " << (sum / ctr) / 100.0 << std::endl;
}

void testIntegerVolts()
{
  uint32_t sum = 0;
  uint16_t ctr = 0;
  iter_readings = adcreadings.begin();
  auto start = std::chrono::high_resolution_clock::now();
  while (iter_readings != adcreadings.end())
  {
    uint32_t volts = calcRMS(5000 / 4, 55.3, 10);
    if (volts == UINT16_MAX)
    {
      break;
    }
    sum += volts;
    ctr++;
    std::cout << "VOLTS: " << (volts / 100.0) << std::endl;
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::micro> diff = end - start;
  std::cout << "Integer Volts Took Time " << diff.count() << " AVERAGE " << ((sum / ctr) / 100.0) << std::endl;
}

void doIRMS(std::string filename)
{
  std::ifstream ifs(filename);
  if (!ifs.is_open())
  {
    std::cout << "failed to open " << filename << '\n';
  }
  else
  {
    std::string time;
    uint16_t adcv;
    while (!ifs.eof())
    {
      ifs >> time >> adcv;
      time[time.length() - 1] = '\0';
      adcreadings.push_back(_adcreading(std::stoul(time), adcv));
    }
    std::cout << "Readings " << adcreadings.size() << std::endl;

    // testDoubleIRMS();
    testIntegerIRMS();
    // testIntegerIRMSTime();
  }

  std::cout << std::endl
            << std::endl
            << std::endl;
  std::cout.flush();
}

void doVOLTS(std::string filename)
{
  std::ifstream ifs(filename);
  if (!ifs.is_open())
  {
    std::cout << "failed to open " << filename << '\n';
  }
  else
  {
    adcreadings.clear();
    std::string time;
    uint16_t adcv;
    while (!ifs.eof())
    {
      ifs >> time >> adcv;
      time[time.length() - 1] = '\0';
      adcreadings.push_back(_adcreading(std::stoul(time), adcv));
    }
    std::cout << "Readings " << adcreadings.size() << std::endl;

    // testDoubleVoltsEmon();
    testIntegerVolts();
    // testIntegerIRMS();
    // testIntegerIRMSTime();
  }

  std::cout << std::endl
            << std::endl
            << std::endl;
  std::cout.flush();
}

int main(int argc, char **argv)
{
  std::cout << "RMS Tests" << std::endl;
  if (argc < 2)
  {
    return 1;
  }
  std::string filename(argv[1]);
  doIRMS(filename);

  std::string filename2(argv[2]);
  doVOLTS(filename2);
  std::this_thread::sleep_for(std::chrono::seconds(2));

  return 0;
}
