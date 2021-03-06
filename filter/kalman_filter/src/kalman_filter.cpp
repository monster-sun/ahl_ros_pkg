#include "kalman_filter/kalman_filter.hpp"
#include "kalman_filter/exception.hpp"

using namespace kf;

void KalmanFilter::setRandomVariables(const NormalDistributionPtr& state,
                                      const NormalDistributionPtr& uncertainty,
                                      const NormalDistributionPtr& msr_noise)
{
  state_       = state;
  uncertainty_ = uncertainty;
  msr_noise_   = msr_noise;

  predicted_state_ = NormalDistributionPtr(new NormalDistribution(state_->getMean(), state_->getVariance()));
}

void KalmanFilter::setLinearModel(const Eigen::MatrixXd& coeff_of_mean,
                                  const Eigen::MatrixXd& coeff_of_ctrl_data,
                                  const Eigen::MatrixXd& coeff_of_msr_data)
{
  if(coeff_of_mean.rows() != coeff_of_mean.cols())
  {
    std::stringstream msg;

    msg << "coeff_of_mean.rows() != coeff_of_mean.cols()" << std::endl
        << "          coeff_of_mean.rows() : " << coeff_of_mean.rows() << std::endl
        << "          coeff_of_mean.cols() : " << coeff_of_mean.cols();

    throw kf::Exception("KalmanFilter::setLinearModel", msg.str());
  }

  A_ = coeff_of_mean;
  B_ = coeff_of_ctrl_data;
  C_ = coeff_of_msr_data;
}

void KalmanFilter::estimate(const Eigen::MatrixXd& ctrl_data, const Eigen::MatrixXd& msr_data, NormalDistributionPtr& state)
{
  Eigen::MatrixXd mean     = A_ * state_->getMean() + B_ * ctrl_data;
  Eigen::MatrixXd variance = A_ * state_->getVariance() * A_.transpose() + uncertainty_->getVariance();
  predicted_state_->set(mean, variance);

  Eigen::MatrixXd kalman_gain = variance * C_.transpose() * (C_ * variance * C_.transpose() + msr_noise_->getVariance()).inverse();

  Eigen::MatrixXd I = Eigen::MatrixXd::Identity(variance.rows(), variance.cols());
  mean = mean + kalman_gain * (msr_data - C_ * mean);
  variance = (I - kalman_gain * C_) * variance;

  state->set(mean, variance);
  state_ = state;
}

void KalmanFilter::checkMatrixSize(const Eigen::MatrixXd& ctrl_data, const Eigen::MatrixXd& msr_data)
{
  std::string src = "KalmanFilter::checkMatrixSize";

  if(A_.cols() != state_->getMean().rows())
  {
    std::stringstream msg;

    msg << "A_.cols() != state_->getMean().rows()" << std::endl
        << "          A_.cols() : " << A_.cols() << std::endl
        << "          state_->getMean().rows() : " << state_->getMean().rows();

    throw kf::Exception(src, msg.str());
  }

  if(B_.cols() != ctrl_data.rows())
  {
    std::stringstream msg;

    msg << "B_.cols() != ctrl_data.rows()" << std::endl
        << "          B_.cols() : " << B_.cols() << std::endl
        << "          ctrl_data.rows() : " << ctrl_data.rows();

    throw kf::Exception(src, msg.str());
  }

  if(A_.cols() != state_->getVariance().rows())
  {
    std::stringstream msg;

    msg << "A_.cols() != state_->getVariance().rows()" << std::endl
        << "          A_.cols() : " << A_.cols() << std::endl
        << "          state_->getVariance().rows() : " << state_->getVariance().rows();

    throw kf::Exception(src, msg.str());
  }

  if(A_.rows() != state_->getVariance().cols())
  {
    std::stringstream msg;

    msg << "A_.rows() != state_->getVariance().cols()" << std::endl
        << "          A_.rows() : " << A_.rows() << std::endl
        << "          state_->getVariance().cols() : " << state_->getVariance().cols();

    throw kf::Exception(src, msg.str());
  }
}
