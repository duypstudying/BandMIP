/*=====================================================================================

    Filename:     Model_Var.cpp

    Description:
        Version:  2.0

    Author:       Peng Lin, peng.lin.csor@gmail.com

    Organization: Shaowei Cai Group

=====================================================================================*/

#include "../utils/global_defs.h"
#include "Model_Var.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

Model_Var::Model_Var(const std::string& p_name,
                     size_t p_idx,
                     bool p_integrality)
    : m_name(p_name), m_idx(p_idx), m_upper_bound(k_inf),
      m_lower_bound(k_default_lower_bound), m_type(Var_Type::real),
      m_requires_integrality(p_integrality)
{
  if (p_integrality)
  {
    m_type = Var_Type::binary;
    m_upper_bound = k_default_integer_upper_bound;
    m_lower_bound = k_default_lower_bound;
  }
}

Model_Var::~Model_Var()
{
  m_con_idx_list.clear();
  m_pos_in_con_list.clear();
}

void Model_Var::set_type(Var_Type p_var_type)
{
  // 与基准版（嵌套 src\src）对齐：类型切到整数/二进制时，对已有边界重新取整
  // （P1 修复：LP 文件先解析 Bounds 再解析 Generals/Binaries，小数边界必须回补取整）。
  if (p_var_type == Var_Type::binary ||
      p_var_type == Var_Type::general_integer)
  {
    m_requires_integrality = true;
    if (m_type == Var_Type::real)
    {
      m_lower_bound = std::ceil(m_lower_bound - k_feas_tolerance);
      m_upper_bound = std::floor(m_upper_bound + k_feas_tolerance);
    }
    if (p_var_type == Var_Type::binary)
    {
      m_lower_bound = std::max(m_lower_bound, 0.0);
      m_upper_bound = std::min(m_upper_bound, 1.0);
    }
  }
  else if (p_var_type == Var_Type::real)
    m_requires_integrality = false;
  m_type = p_var_type;
}

void Model_Var::set_lower_bound(double p_lower_bound)
{
  assert(m_type != Var_Type::fixed);
  if (m_type == Var_Type::real)
    m_lower_bound = p_lower_bound;
  else
    m_lower_bound = std::ceil(p_lower_bound - k_feas_tolerance);
}

void Model_Var::set_upper_bound(double p_upper_bound)
{
  assert(m_type != Var_Type::fixed);
  if (m_type == Var_Type::real)
    m_upper_bound = p_upper_bound;
  else
    m_upper_bound = std::floor(p_upper_bound + k_feas_tolerance);
}

void Model_Var::set_pos_in_con(const size_t term_idx,
                               const size_t pos_in_con)
{
  m_pos_in_con_list[term_idx] = pos_in_con;
}

void Model_Var::set_conflict_value(double p_conflict_value)
{
  m_conflict_value = p_conflict_value;
}

