
#include "CGenet.h"
#include <cassert>

int CGenet::staticID = 0;

/*
 * If the ramet has enough resources to fulfill its minimum requirements,
 * it will "donate" the rest of its resources to a pool that is then equally
 * shared across all the genet's ramets.
 *
 * This is for aboveground
 */
void CGenet::ResshareA()
{
  double sumAuptake=0;
  double MeanAuptake=0;

	for (auto const& ramet_ptr : AllRametList)
	{
		auto ramet = ramet_ptr.lock();
		assert(ramet);

		double AddtoSum = 0;
		double minres = ramet->Traits->mThres * ramet->Ash_disc * ramet->Traits->Gmax * 2;

		AddtoSum = std::max(0.0, ramet->Auptake - minres);

		if (AddtoSum > 0)
		{
			ramet->Auptake = minres;
			sumAuptake += AddtoSum;
		}
	}
	MeanAuptake = sumAuptake / AllRametList.size();

	for (auto const& ramet_ptr : AllRametList)
	{
		auto ramet = ramet_ptr.lock();
		assert(ramet);

		ramet->Auptake += MeanAuptake;
	}
}
/*
 * This is for belowground.
 */
void CGenet::ResshareB()
{
	double sumBuptake = 0;
	double MeanBuptake = 0;

	for (auto const& ramet_ptr : AllRametList)
	{
		auto ramet = ramet_ptr.lock();
		assert(ramet);

		double AddtoSum = 0;
		double minres = ramet->Traits->mThres * ramet->Art_disc * ramet->Traits->Gmax * 2;

		AddtoSum = std::max(0.0, ramet->Buptake - minres);

		if (AddtoSum > 0)
		{
			ramet->Buptake = minres;
			sumBuptake += AddtoSum;
		}
	}

	MeanBuptake = sumBuptake / AllRametList.size();

	for (auto const& ramet_ptr : AllRametList)
	{
		auto ramet = ramet_ptr.lock();
		assert(ramet);

		ramet->Buptake += MeanBuptake;
	}
}

