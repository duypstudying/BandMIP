/*=====================================================================================

    Filename:     explore_unsat_random.cpp

    Description:  MAB (KL-UCB + ε-greedy) based constraint-balancing neighbor.
                  Implements the action selection of the paper
                  "基于多臂老虎机算法的约束冲突型混合整数规划搜索" (Sec. 3.7).

        Version:  2.0

    Author:       Peng Lin, peng.lin.csor@gmail.com

    Organization: Shaowei Cai Group

=====================================================================================*/

#include "../utils/paras.h"
#include "../../utils/global_defs.h"
#include "neighbor.h"
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <random>
#include <vector>

void Neighbor::explore_unsat_random_bm(Neighbor_Ctx& p_ctx)
{
  // 当前模式（AGR/FINE）已在邻域探索前由 Local_Search::update_state() 统一确定，
  // 这里直接读取，避免阈值口径不一致（论文 3.2）。
  p_ctx.isRandom = true;
  p_ctx.mtm_var_idxs.clear();
  p_ctx.mtm_var_deltas.clear();

  // ε 随搜索不断衰减，逐步转向利用；衰减至 <1% 后复位为初始值 80%（论文 3.4）。
  if (p_ctx.decay_period > 0 && p_ctx.RandomStep > 0 &&
      p_ctx.RandomStep % p_ctx.decay_period == 0)
  {
    p_ctx.epsilon *= p_ctx.epsilon_decay;
    if (p_ctx.epsilon < p_ctx.epsilon_min)
      p_ctx.epsilon = p_ctx.epsilon_init;
  }

  size_t unsat_size = p_ctx.m_shared.m_con_unsat_idxs.size();
  if (unsat_size > 0)
  {
    size_t mode = p_ctx.state; // 0 = AGR, 1 = FINE
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    size_t con_idx;
    if (uni(p_ctx.m_rng) < p_ctx.epsilon)
    {
      // 探索：以概率 ε 随机选择一条被违反的约束（论文 3.7）。
      std::uniform_int_distribution<size_t> dist(0, unsat_size - 1);
      con_idx = p_ctx.m_shared.m_con_unsat_idxs[dist(p_ctx.m_rng)];
    }
    else
    {
      // 利用：KL-UCB 选择（论文 3.7）
      //   UCB_m(a) = w_value·Value_m(a) + w_conflict·Conflict(a) + w_kl·KL_term
      double log_total =
          std::log(static_cast<double>(p_ctx.PickConTime[mode]) + 2.0);
      double best_ucb = -std::numeric_limits<double>::infinity();
      double best_value = -std::numeric_limits<double>::infinity();
      size_t best_idx = p_ctx.m_shared.m_con_unsat_idxs[0];
      for (size_t idx = 0; idx < unsat_size; ++idx)
      {
        size_t Idx = p_ctx.m_shared.m_con_unsat_idxs[idx];
        double value = p_ctx.ConQTable[Idx][mode];
        // Conflict(a)：约束内部冲突值 l_c（论文 3.6），使搜索关注缓解冲突的约束
        double conflict =
            p_ctx.m_shared.m_model_manager.con(Idx).con_importance();
        double n = static_cast<double>(p_ctx.PickCon[Idx][mode]) + 1.0;
        // 置信项：鼓励选择次数较少的动作，随 C_m(a) 增大而减小
        double kl_term = std::sqrt(log_total / n);
        double ucb_value = p_ctx.w_value * value +
                           p_ctx.w_conflict * conflict +
                           p_ctx.w_kl * kl_term;
        if (ucb_value > best_ucb)
        {
          best_ucb = ucb_value;
          best_value = value;
          best_idx = Idx;
        }
      }
      // 方差机制（论文 3.7）：当候选动作的 Value 与最优动作接近
      // （|Value(a_i) - Value(a_m)| <= Δ）且奖励方差更小时，替换为更稳定的动作。
      double best_var = p_ctx.dConReward[best_idx][mode];
      for (size_t idx = 0; idx < unsat_size; ++idx)
      {
        size_t Idx = p_ctx.m_shared.m_con_unsat_idxs[idx];
        if (Idx == best_idx)
          continue;
        double value = p_ctx.ConQTable[Idx][mode];
        double var = p_ctx.dConReward[Idx][mode];
        if (std::fabs(value - best_value) <= p_ctx.delta && var < best_var)
        {
          best_idx = Idx;
          best_var = var;
        }
      }
      con_idx = best_idx;
    }

    // 关键：两条分支都必须记录被选中的动作，奖励才能正确归属（论文 3.5）。
    p_ctx.Pick_con_idx = con_idx;

    auto& model_con = p_ctx.m_shared.m_model_manager.con(con_idx);
    for (size_t term_idx = 0; term_idx < model_con.term_num(); ++term_idx)
    {
      size_t var_idx = model_con.var_idx(term_idx);
      double delta;
      if (p_ctx.m_shared.m_con_is_equality[con_idx])
        delta = equality_mixed_tight_operation(
            con_idx, term_idx, var_idx, p_ctx);
      else
        delta = inequality_mixed_tight_operation(
            con_idx, term_idx, var_idx, p_ctx);
      if (tabu_latest(p_ctx, var_idx, delta))
        continue;
      if (std::fabs(delta) < k_zero_tolerance)
        continue;
      p_ctx.m_op_var_idxs.push_back(var_idx);
      p_ctx.m_op_var_deltas.push_back(delta);
      p_ctx.mtm_var_idxs.push_back(var_idx);
      p_ctx.mtm_var_deltas.push_back(delta);
    }
  }

  auto& model_obj = p_ctx.m_shared.m_model_manager.obj();
  if (p_ctx.m_shared.m_is_found_feasible &&
      !p_ctx.m_shared.m_current_obj_breakthrough)
    for (size_t term_idx = 0; term_idx < p_ctx.m_shared.m_obj_var_num;
         ++term_idx)
    {
      size_t var_idx = model_obj.var_idx(term_idx);
      double delta = breakthrough_operation(term_idx, var_idx, p_ctx);
      if (tabu_latest(p_ctx, var_idx, delta))
        continue;
      if (std::fabs(delta) < k_zero_tolerance)
        continue;
      p_ctx.m_op_var_idxs.push_back(var_idx);
      p_ctx.m_op_var_deltas.push_back(delta);
    }
  p_ctx.m_op_size = sample_op(
      m_bms_op, p_ctx.m_op_var_idxs, p_ctx.m_op_var_deltas, p_ctx);
  p_ctx.RandomStep++; // 选择次数加 1
}
