/*=====================================================================================

    Filename:     Model_Con.h

    Description:
        Version:  2.0

    Author:       Peng Lin, peng.lin.csor@gmail.com

    Organization: Shaowei Cai Group

=====================================================================================*/

#pragma once
#include "../utils/global_defs.h"
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

class Model_Manager;
class Model_Con
{
private:
  std::string m_name;

  size_t m_idx;

  bool m_is_equality;

  bool m_is_greater;

  bool m_is_demand;

  bool m_is_supply;

  int m_con_type;

  // 约束内部冲突值 l_c（论文 3.6）：其包含变量冲突度的均值，取值域为 (0,1)。
  // 必须使用 double，否则会被截断为 0。
  double m_con_importance;

  std::vector<double> m_coeff_list;

  std::vector<size_t> m_var_idx_list;

  std::vector<size_t> m_pos_in_var_list;

  double m_rhs;

  bool m_mark_inferred_sat;

  std::vector<Con_Type> m_types;

public:
  Model_Con(const std::string& p_name,
            const size_t p_idx,
            const char p_symbol = '<');

  ~Model_Con();

  void convert_greater_to_less();

  void convert_equality_to_less();

  void delete_term_at(const size_t p_term_idx,
                      double var_value,
                      Model_Manager* p_model_manager);

  inline void mark_inferred_sat();

  inline void add_var(const size_t p_var_idx,
                      const double p_coeff,
                      const size_t p_pos_in_var);

  inline void set_rhs(double p_rhs);

  inline void set_coeff(size_t p_term_idx, double p_coeff);

  inline bool verify_empty_sat() const;

  inline void add_type(Con_Type p_type);

  inline void set_con_type(int type);

  inline void set_con_importance(double p_con_importance);

  inline void set_demand(int flag);

  inline void set_supply(int flag);

  inline size_t term_num() const;

  inline const std::string& name() const;

  inline double unique_coeff() const;

  inline size_t unique_var_idx() const;

  inline bool is_equality() const;

  inline bool is_demand() const;

  inline bool is_supply() const;

  inline int con_type() const;

  inline double con_importance() const;

  inline double rhs() const;

  inline double coeff(const size_t p_term_idx) const;

  inline size_t var_idx(const size_t p_term_idx) const;

  inline const std::vector<size_t>& var_idx_set() const;

  inline const std::vector<double>& coeff_set() const;

  inline bool has_type(Con_Type p_type) const;

  inline const std::vector<Con_Type>& get_types() const;

  inline size_t idx() const;

  inline bool is_greater() const;

  inline bool is_inferred_sat() const;
};

inline size_t Model_Con::term_num() const
{
  return m_coeff_list.size();
}

inline const std::string& Model_Con::name() const
{
  return m_name;
}

inline double Model_Con::unique_coeff() const
{
  return m_coeff_list[0];
}

inline size_t Model_Con::unique_var_idx() const
{
  return m_var_idx_list[0];
}

inline bool Model_Con::is_equality() const
{
  return m_is_equality;
}

inline bool Model_Con::is_demand() const
{
  return m_is_demand;
}

inline void Model_Con::set_demand(int flag){
  m_is_demand=flag;
}

inline void Model_Con::set_supply(int flag){
  m_is_supply=flag;
}

inline bool Model_Con::is_supply() const
{
  return m_is_supply;
}

inline int Model_Con::con_type() const
{
  return m_con_type;
}

inline void Model_Con::set_con_importance(double p_con_importance)
{
  m_con_importance=p_con_importance;
}


inline double Model_Con::rhs() const
{
  return m_rhs;
}

inline double Model_Con::coeff(const size_t p_term_idx) const
{
  return m_coeff_list[p_term_idx];
}

inline size_t Model_Con::var_idx(const size_t p_term_idx) const
{
  return m_var_idx_list[p_term_idx];
}

inline const std::vector<size_t>& Model_Con::var_idx_set() const
{
  return m_var_idx_list;
}

inline const std::vector<double>& Model_Con::coeff_set() const
{
  return m_coeff_list;
}

inline bool Model_Con::has_type(Con_Type p_type) const
{
  for (const auto& type : m_types)
  {
    if (type == p_type)
      return true;
  }
  return false;
}

inline const std::vector<Con_Type>& Model_Con::get_types() const
{
  return m_types;
}

inline size_t Model_Con::idx() const
{
  return m_idx;
}

inline bool Model_Con::is_greater() const
{
  return m_is_greater;
}

inline bool Model_Con::is_inferred_sat() const
{
  return m_mark_inferred_sat;
}

inline bool Model_Con::verify_empty_sat() const
{
  if ((!m_is_equality && m_rhs + k_feas_tolerance >= 0) ||
      (m_is_equality && std::fabs(m_rhs) <= k_feas_tolerance))
    return true;
  return false;
}

inline void Model_Con::add_type(Con_Type p_type)
{
  if (!has_type(p_type))
    m_types.push_back(p_type);
}

inline void Model_Con::set_rhs(double p_rhs)
{
  m_rhs = p_rhs;
}

inline void Model_Con::set_coeff(size_t p_term_idx, double p_coeff)
{
  m_coeff_list[p_term_idx] = p_coeff;
}

inline void Model_Con::set_con_type(int type)
{
  m_con_type=type;
}

inline double Model_Con::con_importance() const
{
  return m_con_importance;
}

inline void Model_Con::add_var(const size_t p_var_idx,
                               const double p_coeff,
                               const size_t p_pos_in_var)
{
  m_var_idx_list.push_back(p_var_idx);
  m_coeff_list.push_back(p_coeff);
  m_pos_in_var_list.push_back(p_pos_in_var);
}

inline void Model_Con::mark_inferred_sat()
{
  m_mark_inferred_sat = true;
}
