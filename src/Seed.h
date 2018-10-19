
#ifndef SRC_SEED_H_
#define SRC_SEED_H_

#include <memory>

class Cell;
class Plant;
class Traits;

class Seed
{
    protected:
       Cell* cell;

    public:
       std::unique_ptr<Traits> traits;

       double mass;
       double pEstab;
       int age;
       bool toBeRemoved;

       Seed(const std::shared_ptr<Plant> & plant, Cell* cell);
       Seed(std::string PFT_ID, Cell* cell, const double estab);

       Cell* getCell() { return cell; }

       inline static bool GetSeedRemove(const std::unique_ptr<Seed> & s) { return s->toBeRemoved; }
};

#endif
