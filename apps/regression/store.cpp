#include "store.h"
#include "linear_model_helper.h"
#include <poincare/preferences.h>
#include <assert.h>
#include <float.h>
#include <cmath>
#include <string.h>
#include <algorithm>

using namespace Shared;

namespace Regression {

static_assert(Model::k_numberOfModels == 10, "Number of models changed, Regression::Store() needs to adapt");
static_assert(Store::k_numberOfSeries == 3, "Number of series changed, Regression::Store() needs to adapt (m_seriesChecksum)");

Store::Store() :
  InteractiveCurveViewRange(),
  DoublePairStore(),
  m_angleUnit(Poincare::Preferences::AngleUnit::Degree)
{
  resetMemoization();
}

void Store::reset() {
  deleteAllPairs();
  resetMemoization();
}

void Store::tidy() {
  for (int i = 0; i < Model::k_numberOfModels; i++) {
    regressionModel(i)->tidy();
  }
}

/* Regressions */
void Store::setSeriesRegressionType(int series, Model::Type type) {
  assert(series >= 0 && series < k_numberOfSeries);
  if (m_regressionTypes[series] != type) {
    m_regressionTypes[series] = type;
    m_regressionChanged[series] = true;
  }
}

/* Dots */

int Store::closestVerticalDot(int direction, double x, double y, int currentSeries, int currentDot, int * nextSeries, Poincare::Context * globalContext) {
  double nextX = INFINITY;
  double nextY = INFINITY;
  int selectedDot = -1;
  for (int series = 0; series < k_numberOfSeries; series++) {
    if (seriesIsEmpty(series) || (currentDot >= 0 && currentSeries == series)) {
      /* If the currentDot is valid, the next series should not be the current
       * series */
      continue;
    }
    int numberOfPoints = numberOfPairsOfSeries(series);
    for (int i = 0; i <= numberOfPoints; i++) {
      double currentX = i < numberOfPoints ? m_data[series][0][i] : meanOfColumn(series, 0);
      double currentY = i < numberOfPoints ? m_data[series][1][i] : meanOfColumn(series, 1);
      if (xMin() <= currentX && currentX <= xMax() // The next dot is within the window abscissa bounds
          && (std::fabs(currentX - x) <= std::fabs(nextX - x)) // The next dot is the closest to x in abscissa
          && ((currentY > y && direction > 0) // The next dot is above/under y
            || (currentY < y && direction < 0)
            || (currentY == y
              && ((currentDot < 0 && direction > 0)|| ((direction < 0) == (series > currentSeries)))))
          && (nextX != currentX // Edge case: if 2 dots have the same abscissa but different ordinates
            || ((currentY <= nextY) == (direction > 0))))
      {
        nextX = currentX;
        nextY = currentY;
        selectedDot = i;
        *nextSeries = series;
      }
    }
  }
  return selectedDot;
}

int Store::nextDot(int series, int direction, int dot) {
  float nextX = INFINITY;
  int selectedDot = -1;
  double meanX = meanOfColumn(series, 0);
  float x = meanX;
  if (dot >= 0 && dot < numberOfPairsOfSeries(series)) {
    x = get(series, 0, dot);
  }
  /* We have to scan the Store in opposite ways for the 2 directions to ensure to
   * select all dots (even with equal abscissa) */
  if (direction > 0) {
    for (int index = 0; index < numberOfPairsOfSeries(series); index++) {
      /* The conditions to test are in this order:
       * - the next dot is the closest one in abscissa to x
       * - the next dot is not the same as the selected one
       * - the next dot is at the right of the selected one */
      if (std::fabs(m_data[series][0][index] - x) < std::fabs(nextX - x) &&
          (index != dot) &&
          (m_data[series][0][index] >= x)) {
        // Handle edge case: 2 dots have same abscissa
        if (m_data[series][0][index] != x || (index > dot)) {
          nextX = m_data[series][0][index];
          selectedDot = index;
        }
      }
    }
    // Compare with the mean dot
    if (std::fabs(meanX - x) < std::fabs(nextX - x) &&
        (numberOfPairsOfSeries(series) != dot) &&
        (meanX >= x)) {
      if (meanX != x || (numberOfPairsOfSeries(series) > dot)) {
        selectedDot = numberOfPairsOfSeries(series);
      }
    }
  } else {
    // Compare with the mean dot
    if (std::fabs(meanX - x) < std::fabs(nextX - x) &&
        (numberOfPairsOfSeries(series) != dot) &&
        (meanX <= x)) {
      if ((meanX != x) || (numberOfPairsOfSeries(series) < dot)) {
        nextX = meanX;
        selectedDot = numberOfPairsOfSeries(series);
      }
    }
    for (int index = numberOfPairsOfSeries(series)-1; index >= 0; index--) {
      if (std::fabs(m_data[series][0][index] - x) < std::fabs(nextX - x) &&
          (index != dot) &&
          (m_data[series][0][index] <= x)) {
        // Handle edge case: 2 dots have same abscissa
        if (m_data[series][0][index] != x || (index < dot)) {
          nextX = m_data[series][0][index];
          selectedDot = index;
        }
      }
    }
  }
  return selectedDot;
}

/* Window */

void Store::setDefault() {
  setZoomNormalize(false);

  float xMin, xMax, yMin, yMax;
  float mins[k_numberOfSeries], maxs[k_numberOfSeries];
  for (int series = 0; series < k_numberOfSeries; series++) {
    bool empty = seriesIsEmpty(series);
    mins[series] = empty ? NAN : minValueOfColumn(series, 0);
    maxs[series] = empty ? NAN : maxValueOfColumn(series, 0);
  }
  Poincare::Zoom::CombineRanges(k_numberOfSeries, mins, maxs, &xMin, &xMax);

  for (int series = 0; series < k_numberOfSeries; series++) {
    bool empty = seriesIsEmpty(series);
    mins[series] = empty ? NAN : minValueOfColumn(series, 1);
    maxs[series] = empty ? NAN : maxValueOfColumn(series, 1);
  }
  Poincare::Zoom::CombineRanges(k_numberOfSeries, mins, maxs, &yMin, &yMax);

  Poincare::Zoom::SanitizeRange(&xMin, &xMax, &yMin, &yMax, NormalYXRatio());

  m_xRange.setMin(xMin);
  m_xRange.setMax(xMax);
  m_yRange.setMin(yMin);
  m_yRange.setMax(yMax);
  bool revertToOrthonormal = shouldBeNormalized();

  float range = xMax - xMin;
  setXMin(xMin - k_displayHorizontalMarginRatio * range);
  setXMax(xMax + k_displayHorizontalMarginRatio * range);

  range = yMax - yMin;
  setYMin(roundLimit(m_delegate->addMargin(yMin, range, true, true ), range, true));
  setYMax(roundLimit(m_delegate->addMargin(yMax, range, true, false), range, false));

  if (revertToOrthonormal) {
    normalize();
  }
  setZoomAuto(true);
}

/* Series */

bool Store::seriesIsEmpty(int series) const {
  return numberOfPairsOfSeries(series) < 2;
}

/* Calculations */

void Store::updateCoefficients(int series, Poincare::Context * globalContext) {
  assert(series >= 0 && series <= k_numberOfSeries);
  assert(!seriesIsEmpty(series));
  uint32_t storeChecksumSeries = storeChecksumForSeries(series);
  Poincare::Preferences::AngleUnit currentAngleUnit = Poincare::Preferences::sharedPreferences()->angleUnit();
  if (m_angleUnit != currentAngleUnit) {
    m_angleUnit = currentAngleUnit;
    for (int i = 0; i < k_numberOfSeries; i++) {
      if (m_regressionTypes[i] == Model::Type::Trigonometric) {
        /* TODO : Assuming regression should be independent of angleUnit,
         * coefficients b and c could just be converted to the new angle unit.*/
        m_regressionChanged[i] = true;
      }
    }
  }
  if (m_regressionChanged[series] || (m_seriesChecksum[series] != storeChecksumSeries)) {
    Model * seriesModel = modelForSeries(series);
    seriesModel->fit(this, series, m_regressionCoefficients[series], globalContext);
    m_regressionChanged[series] = false;
    m_seriesChecksum[series] = storeChecksumSeries;
    /* m_determinationCoefficient must be updated after m_seriesChecksum and m_regressionChanged
     * updates to avoid infinite recursive calls as computeDeterminationCoefficient calls
     * yValueForXValue which calls coefficientsForSeries which calls updateCoefficients */
    m_determinationCoefficient[series] = computeDeterminationCoefficient(series, globalContext);
  }
}

double * Store::coefficientsForSeries(int series, Poincare::Context * globalContext) {
  updateCoefficients(series, globalContext);
  return m_regressionCoefficients[series];
}

double Store::determinationCoefficientForSeries(int series, Poincare::Context * globalContext) {
  /* Returns the Determination coefficient (R2).
   * It will be updated if the regression has been updated */
  updateCoefficients(series, globalContext);
  return m_determinationCoefficient[series];
}

double Store::doubleCastedNumberOfPairsOfSeries(int series) const {
  return DoublePairStore::numberOfPairsOfSeries(series);
}

void Store::resetMemoization() {
  assert(((int)Model::Type::Linear) == 0);
  memset(m_seriesChecksum, 0, sizeof(m_seriesChecksum));
  memset(m_regressionTypes, 0, sizeof(m_regressionTypes));
  memset(m_regressionChanged, 0, sizeof(m_regressionChanged));
}

float Store::maxValueOfColumn(int series, int i) const {
  float maxColumn = -FLT_MAX;
  for (int k = 0; k < numberOfPairsOfSeries(series); k++) {
    maxColumn = std::max<float>(maxColumn, m_data[series][i][k]);
  }
  return maxColumn;
}

float Store::minValueOfColumn(int series, int i) const {
  float minColumn = FLT_MAX;
  for (int k = 0; k < numberOfPairsOfSeries(series); k++) {
    minColumn = std::min<float>(minColumn, m_data[series][i][k]);
  }
  return minColumn;
}

double Store::squaredOffsettedValueSumOfColumn(int series, int i, bool lnOfSeries, double offset) const {
  double result = 0;
  const int numberOfPairs = numberOfPairsOfSeries(series);
  for (int k = 0; k < numberOfPairs; k++) {
    double value = m_data[series][i][k];
    if (lnOfSeries) {
      value = log(value);
    }
    value -= offset;
    result += value * value;
  }
  return result;
}

double Store::squaredValueSumOfColumn(int series, int i, bool lnOfSeries) const {
  return squaredOffsettedValueSumOfColumn(series, i, lnOfSeries, 0.0);
}

double Store::columnProductSum(int series, bool lnOfSeries) const {
  double result = 0;
  for (int k = 0; k < numberOfPairsOfSeries(series); k++) {
    double value0 = m_data[series][0][k];
    double value1 = m_data[series][1][k];
    if (lnOfSeries) {
      value0 = log(value0);
      value1 = log(value1);
    }
    result += value0 * value1;
  }
  return result;
}

double Store::meanOfColumn(int series, int i, bool lnOfSeries) const {
  return numberOfPairsOfSeries(series) == 0 ? 0 : sumOfColumn(series, i, lnOfSeries) / numberOfPairsOfSeries(series);
}

double Store::varianceOfColumn(int series, int i, bool lnOfSeries) const {
  /* We use the Var(X) = E[(X-E[X])^2] definition instead of Var(X) = E[X^2] - E[X]^2
   * to ensure a positive result and to minimize rounding errors */
  double mean = meanOfColumn(series, i, lnOfSeries);
  return squaredOffsettedValueSumOfColumn(series, i, lnOfSeries, mean)/numberOfPairsOfSeries(series);
}

double Store::standardDeviationOfColumn(int series, int i, bool lnOfSeries) const {
  return std::sqrt(varianceOfColumn(series, i, lnOfSeries));
}

double Store::covariance(int series, bool lnOfSeries) const {
  double mean0 = meanOfColumn(series, 0, lnOfSeries);
  double mean1 = meanOfColumn(series, 1, lnOfSeries);
  return columnProductSum(series, lnOfSeries)/numberOfPairsOfSeries(series) - mean0 * mean1;
}

double Store::slope(int series, bool lnOfSeries) const {
  return LinearModelHelper::Slope(covariance(series, lnOfSeries), varianceOfColumn(series, 0, lnOfSeries));
}

double Store::yIntercept(int series, bool lnOfSeries) const {
  return LinearModelHelper::YIntercept(meanOfColumn(series, 1, lnOfSeries), meanOfColumn(series, 0, lnOfSeries), slope(series, lnOfSeries));
}

double Store::yValueForXValue(int series, double x, Poincare::Context * globalContext) {
  Model * model = regressionModel((int)m_regressionTypes[series]);
  double * coefficients = coefficientsForSeries(series, globalContext);
  return model->evaluate(coefficients, x);
}

double Store::xValueForYValue(int series, double y, Poincare::Context * globalContext) {
  Model * model = regressionModel((int)m_regressionTypes[series]);
  double * coefficients = coefficientsForSeries(series, globalContext);
  return model->levelSet(coefficients, xMin(), xGridUnit() / 10.0, xMax(), y, globalContext);
}

double Store::correlationCoefficient(int series) const {
  /* Returns the correlation coefficient (R) between the series X and Y.
   * In non-linear regressions, its square is different from the determinationCoefficient
   * It is usually displayed in linear regressions only to avoid confusion */
  double v0 = varianceOfColumn(series, 0);
  double v1 = varianceOfColumn(series, 1);
  return (v0 == 0.0 || v1 == 0.0) ? 1.0 : covariance(series) / std::sqrt(v0 * v1);
}

double Store::computeDeterminationCoefficient(int series, Poincare::Context * globalContext) {
  /* Computes and returns the determination coefficient (R2) of the regression.
   * For linear regressions, it is equal to the square of the correlation
   * coefficient between the series Y and the evaluated values.
   * With proportional regression or badly fitted models, R2 can technically be
   * negative. R2<0 means that the regression is less effective than a
   * constant set to the series average. It should not happen with regression
   * models that can fit a constant observation. */
  // Residual sum of squares
  double ssr = 0;
  // Total sum of squares
  double sst = 0;
  double mean = meanOfColumn(series, 1);
  const int numberOfPairs = numberOfPairsOfSeries(series);
  for (int k = 0; k < numberOfPairs; k++) {
    // Difference between the observation and the estimated value of the model
    double evaluation = yValueForXValue(series, m_data[series][0][k], globalContext);
    if (std::isnan(evaluation) || std::isinf(evaluation)) {
      // Data Not Suitable for evaluation
      return NAN;
    }
    double residual = m_data[series][1][k] - evaluation;
    ssr += residual * residual;
    // Difference between the observation and the overall observations mean
    double difference = m_data[series][1][k] - mean;
    sst += difference * difference;
  }
  if (sst == 0.0) {
    /* Observation was constant, r2 is undefined. Return 1 if estimations
     * exactly matched observations. 0 is usually returned otherwise. */
    return (ssr <= DBL_EPSILON) ? 1.0 : 0.0;
  }
  double r2 = 1.0 - ssr / sst;
  // Check if regression fit was optimal.
  // TODO : Optimize regression fitting so that r2 cannot be negative.
  // assert(r2 >= 0 || seriesRegressionType(series) == Model::Type::Proportional);
  return r2;
}

Model * Store::regressionModel(int index) {
  Model * models[Model::k_numberOfModels] = {&m_linearModel, &m_proportionalModel, &m_quadraticModel, &m_cubicModel, &m_quarticModel, &m_logarithmicModel, &m_exponentialModel, &m_powerModel, &m_trigonometricModel, &m_logisticModel};
  return models[index];
}

}
