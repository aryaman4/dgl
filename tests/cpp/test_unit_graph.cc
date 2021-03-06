/*!
 *  Copyright (c) 2019 by Contributors
 * \file test_unit_graph.cc
 * \brief Test UnitGraph
 */
#include <gtest/gtest.h>
#include <dgl/array.h>
#include <vector>
#include <dgl/immutable_graph.h>
#include "./common.h"
#include "./../src/graph/heterograph.h"
#include "../../src/graph/unit_graph.h"

using namespace dgl;
using namespace dgl::aten;
using namespace dgl::runtime;

template <typename IdType>
aten::CSRMatrix CSR1(DLContext ctx) {
  /*
   * G = [[0, 0, 1],
   *      [1, 0, 1],
   *      [0, 1, 0],
   *      [1, 0, 1]]
   */
  IdArray g_indptr =
    aten::VecToIdArray(std::vector<IdType>({0, 1, 3, 4, 6}), sizeof(IdType)*8, CTX);
  IdArray g_indices =
    aten::VecToIdArray(std::vector<IdType>({2, 0, 2, 1, 0, 2}), sizeof(IdType)*8, CTX);

  const aten::CSRMatrix &csr_a = aten::CSRMatrix(
    4,
    3,
    g_indptr,
    g_indices,
    aten::NullArray(),
    false);
  return csr_a;
}

template aten::CSRMatrix CSR1<int32_t>(DLContext ctx);
template aten::CSRMatrix CSR1<int64_t>(DLContext ctx);

template <typename IdType>
aten::COOMatrix COO1(DLContext ctx) {
  /*
   * G = [[1, 1, 0],
   *      [0, 1, 0]]
   */
  IdArray g_row =
    aten::VecToIdArray(std::vector<IdType>({0, 0, 1}), sizeof(IdType)*8, CTX);
  IdArray g_col =
    aten::VecToIdArray(std::vector<IdType>({0, 1, 1}), sizeof(IdType)*8, CTX);
  const aten::COOMatrix &coo = aten::COOMatrix(
    2,
    3,
    g_row,
    g_col,
    aten::NullArray(),
    true,
    true);

  return coo;
}

template aten::COOMatrix COO1<int32_t>(DLContext ctx);
template aten::COOMatrix COO1<int64_t>(DLContext ctx);

template <typename IdType>
void _TestUnitGraph(DLContext ctx) {
  const aten::CSRMatrix &csr = CSR1<IdType>(ctx);
  const aten::COOMatrix &coo = COO1<IdType>(ctx);

  auto hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSC(2, csr, SparseFormat::kAny));
  UnitGraphPtr g = hg->relation_graphs()[0];
  ASSERT_EQ(g->GetFormatInUse(), 4);
  
  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSR(2, csr, SparseFormat::kAny));
  g = hg->relation_graphs()[0];
  ASSERT_EQ(g->GetFormatInUse(), 2);

  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCOO(2, coo, SparseFormat::kAny));
  g = hg->relation_graphs()[0];
  ASSERT_EQ(g->GetFormatInUse(), 1);

  auto src = VecToIdArray<int64_t>({1, 2, 5, 3});
  auto dst = VecToIdArray<int64_t>({1, 6, 2, 6});
  auto mg = std::dynamic_pointer_cast<UnitGraph>(
      dgl::UnitGraph::CreateFromCOO(2, 9, 8, src, dst, dgl::SparseFormat::kCOO));
  ASSERT_EQ(mg->GetFormatInUse(), 1);
  auto hmg = dgl::UnitGraph::CreateFromCOO(1, 8, 8, src, dst, dgl::SparseFormat::kCOO);
  auto img = std::dynamic_pointer_cast<ImmutableGraph>(hmg->AsImmutableGraph());
  ASSERT_TRUE(img != nullptr);
  mg = std::dynamic_pointer_cast<UnitGraph>(
      dgl::UnitGraph::CreateFromCOO(2, 9, 8, src, dst, dgl::SparseFormat::kCSR));
  ASSERT_EQ(mg->GetFormatInUse(), 2);
  hmg = dgl::UnitGraph::CreateFromCOO(1, 8, 8, src, dst, dgl::SparseFormat::kCSR);
  img = std::dynamic_pointer_cast<ImmutableGraph>(hmg->AsImmutableGraph());
  ASSERT_TRUE(img != nullptr);
  mg = std::dynamic_pointer_cast<UnitGraph>(
      dgl::UnitGraph::CreateFromCOO(2, 9, 8, src, dst, dgl::SparseFormat::kCSC));
  ASSERT_EQ(mg->GetFormatInUse(), 4);
  hmg = dgl::UnitGraph::CreateFromCOO(1, 8, 8, src, dst, dgl::SparseFormat::kCSC);
  img = std::dynamic_pointer_cast<ImmutableGraph>(hmg->AsImmutableGraph());
  ASSERT_TRUE(img != nullptr);

  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSC(2, csr, SparseFormat::kAuto));
  g = hg->relation_graphs()[0];
  ASSERT_EQ(g->GetFormatInUse(), 4);

  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSR(2, csr, SparseFormat::kAuto));
  g = hg->relation_graphs()[0];
  ASSERT_EQ(g->GetFormatInUse(), 2);

  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCOO(2, coo, SparseFormat::kAuto));
  g = hg->relation_graphs()[0];
  ASSERT_EQ(g->GetFormatInUse(), 1);
}

template <typename IdType>
void _TestUnitGraph_GetInCSR(DLContext ctx) {
  const aten::CSRMatrix &csr = CSR1<IdType>(ctx);
  const aten::COOMatrix &coo = COO1<IdType>(ctx);

  auto hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSC(2, csr, SparseFormat::kAny));
  UnitGraphPtr g = hg->relation_graphs()[0];
  auto in_csr_matrix = g->GetCSCMatrix(0);
  ASSERT_EQ(in_csr_matrix.num_rows, csr.num_rows);
  ASSERT_EQ(in_csr_matrix.num_cols, csr.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 4);

  // test out csr
  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSR(2, csr, SparseFormat::kAny));
  g = hg->relation_graphs()[0];
  UnitGraphPtr g_ptr = std::dynamic_pointer_cast<UnitGraph>(g->GetGraphInFormat(SparseFormat::kCSC));
  in_csr_matrix = g_ptr->GetCSCMatrix(0);
  ASSERT_EQ(in_csr_matrix.num_cols, csr.num_rows);
  ASSERT_EQ(in_csr_matrix.num_rows, csr.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 2);
  in_csr_matrix = g->GetCSCMatrix(0);
  ASSERT_EQ(in_csr_matrix.num_cols, csr.num_rows);
  ASSERT_EQ(in_csr_matrix.num_rows, csr.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 6);

  // test out coo
  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCOO(2, coo, SparseFormat::kAny));
  g = hg->relation_graphs()[0];
  g_ptr = std::dynamic_pointer_cast<UnitGraph>(g->GetGraphInFormat(SparseFormat::kCSC));
  in_csr_matrix = g_ptr->GetCSCMatrix(0);
  ASSERT_EQ(in_csr_matrix.num_cols, coo.num_rows);
  ASSERT_EQ(in_csr_matrix.num_rows, coo.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 1);

  in_csr_matrix = g->GetCSCMatrix(0);
  ASSERT_EQ(in_csr_matrix.num_cols, coo.num_rows);
  ASSERT_EQ(in_csr_matrix.num_rows, coo.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 5);
}

template <typename IdType>
void _TestUnitGraph_GetOutCSR(DLContext ctx) {
  const aten::CSRMatrix &csr = CSR1<IdType>(ctx);
  const aten::COOMatrix &coo = COO1<IdType>(ctx);

  auto hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSC(2, csr, SparseFormat::kAny));
  UnitGraphPtr g = hg->relation_graphs()[0];
  UnitGraphPtr g_ptr = std::dynamic_pointer_cast<UnitGraph>(g->GetGraphInFormat(SparseFormat::kCSR));
  auto out_csr_matrix = g_ptr->GetCSRMatrix(0);
  ASSERT_EQ(out_csr_matrix.num_cols, csr.num_rows);
  ASSERT_EQ(out_csr_matrix.num_rows, csr.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 4);
  out_csr_matrix = g->GetCSRMatrix(0);
  ASSERT_EQ(out_csr_matrix.num_cols, csr.num_rows);
  ASSERT_EQ(out_csr_matrix.num_rows, csr.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 6);

  // test out csr
  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSR(2, csr, SparseFormat::kAny));
  g = hg->relation_graphs()[0];
  out_csr_matrix = g->GetCSRMatrix(0);
  ASSERT_EQ(out_csr_matrix.num_rows, csr.num_rows);
  ASSERT_EQ(out_csr_matrix.num_cols, csr.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 2);

  // test out coo
  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCOO(2, coo, SparseFormat::kAny));
  g = hg->relation_graphs()[0];
  g_ptr = std::dynamic_pointer_cast<UnitGraph>(g->GetGraphInFormat(SparseFormat::kCSR));
  out_csr_matrix = g_ptr->GetCSRMatrix(0);
  ASSERT_EQ(out_csr_matrix.num_rows, coo.num_rows);
  ASSERT_EQ(out_csr_matrix.num_cols, coo.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 1);

  out_csr_matrix = g->GetCSRMatrix(0);
  ASSERT_EQ(out_csr_matrix.num_rows, coo.num_rows);
  ASSERT_EQ(out_csr_matrix.num_cols, coo.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 3);
}

template <typename IdType>
void _TestUnitGraph_GetCOO(DLContext ctx) {
  const aten::CSRMatrix &csr = CSR1<IdType>(ctx);
  const aten::COOMatrix &coo = COO1<IdType>(ctx);

  auto hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSC(2, csr, SparseFormat::kAny));
  UnitGraphPtr g = hg->relation_graphs()[0];
  UnitGraphPtr g_ptr = std::dynamic_pointer_cast<UnitGraph>(g->GetGraphInFormat(SparseFormat::kCOO));
  auto out_coo_matrix = g_ptr->GetCOOMatrix(0);
  ASSERT_EQ(out_coo_matrix.num_cols, csr.num_rows);
  ASSERT_EQ(out_coo_matrix.num_rows, csr.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 4);
  out_coo_matrix = g->GetCOOMatrix(0);
  ASSERT_EQ(out_coo_matrix.num_cols, csr.num_rows);
  ASSERT_EQ(out_coo_matrix.num_rows, csr.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 5);

  // test out csr
  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSR(2, csr, SparseFormat::kAny));
  g = hg->relation_graphs()[0];
  g_ptr = std::dynamic_pointer_cast<UnitGraph>(g->GetGraphInFormat(SparseFormat::kCOO));
  out_coo_matrix = g_ptr->GetCOOMatrix(0);
  ASSERT_EQ(out_coo_matrix.num_rows, csr.num_rows);
  ASSERT_EQ(out_coo_matrix.num_cols, csr.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 2);
  out_coo_matrix = g->GetCOOMatrix(0);
  ASSERT_EQ(out_coo_matrix.num_rows, csr.num_rows);
  ASSERT_EQ(out_coo_matrix.num_cols, csr.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 3);

  // test out coo
  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCOO(2, coo, SparseFormat::kAny));
  g = hg->relation_graphs()[0];
  out_coo_matrix = g->GetCOOMatrix(0);
  ASSERT_EQ(out_coo_matrix.num_rows, coo.num_rows);
  ASSERT_EQ(out_coo_matrix.num_cols, coo.num_cols);
  ASSERT_EQ(g->GetFormatInUse(), 1);
}

template <typename IdType>
void _TestUnitGraph_Reserve(DLContext ctx) {
  const aten::CSRMatrix &csr = CSR1<IdType>(ctx);
  const aten::COOMatrix &coo = COO1<IdType>(ctx);

  auto hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSC(2, csr, SparseFormat::kAny));
  UnitGraphPtr g = hg->relation_graphs()[0];
  ASSERT_EQ(g->GetFormatInUse(), 4);
  UnitGraphPtr r_g = g->Reverse();
  ASSERT_EQ(r_g->GetFormatInUse(), 2);
  aten::CSRMatrix g_in_csr = g->GetCSCMatrix(0);
  aten::CSRMatrix r_g_out_csr = r_g->GetCSRMatrix(0);
  ASSERT_TRUE(g_in_csr.indptr->data == r_g_out_csr.indptr->data);
  ASSERT_TRUE(g_in_csr.indices->data == r_g_out_csr.indices->data);
  aten::CSRMatrix g_out_csr = g->GetCSRMatrix(0);
  ASSERT_EQ(g->GetFormatInUse(), 6);
  ASSERT_EQ(r_g->GetFormatInUse(), 6);
  aten::CSRMatrix r_g_in_csr = r_g->GetCSCMatrix(0);
  ASSERT_TRUE(g_out_csr.indptr->data == r_g_in_csr.indptr->data);
  ASSERT_TRUE(g_out_csr.indices->data == r_g_in_csr.indices->data);
  aten::COOMatrix g_coo = g->GetCOOMatrix(0);
  ASSERT_EQ(g->GetFormatInUse(), 7);
  ASSERT_EQ(r_g->GetFormatInUse(), 6);
  aten::COOMatrix r_g_coo = r_g->GetCOOMatrix(0);
  ASSERT_EQ(r_g->GetFormatInUse(), 7);
  ASSERT_EQ(g_coo.num_rows, r_g_coo.num_cols);
  ASSERT_EQ(g_coo.num_cols, r_g_coo.num_rows);
  ASSERT_TRUE(ArrayEQ<IdType>(g_coo.row, r_g_coo.col));
  ASSERT_TRUE(ArrayEQ<IdType>(g_coo.col, r_g_coo.row));

  // test out csr
  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCSR(2, csr, SparseFormat::kAny));
  g = hg->relation_graphs()[0];
  ASSERT_EQ(g->GetFormatInUse(), 2);
  r_g = g->Reverse();
  ASSERT_EQ(r_g->GetFormatInUse(), 4);
  g_out_csr = g->GetCSRMatrix(0);
  r_g_in_csr = r_g->GetCSCMatrix(0);
  ASSERT_TRUE(g_out_csr.indptr->data == r_g_in_csr.indptr->data);
  ASSERT_TRUE(g_out_csr.indices->data == r_g_in_csr.indices->data);
  g_in_csr = g->GetCSCMatrix(0);
  ASSERT_EQ(g->GetFormatInUse(), 6);
  ASSERT_EQ(r_g->GetFormatInUse(), 6);
  r_g_out_csr = r_g->GetCSRMatrix(0);
  ASSERT_TRUE(g_in_csr.indptr->data == r_g_out_csr.indptr->data);
  ASSERT_TRUE(g_in_csr.indices->data == r_g_out_csr.indices->data);
  g_coo = g->GetCOOMatrix(0);
  ASSERT_EQ(g->GetFormatInUse(), 7);
  ASSERT_EQ(r_g->GetFormatInUse(), 6);
  r_g_coo = r_g->GetCOOMatrix(0);
  ASSERT_EQ(r_g->GetFormatInUse(), 7);
  ASSERT_EQ(g_coo.num_rows, r_g_coo.num_cols);
  ASSERT_EQ(g_coo.num_cols, r_g_coo.num_rows);
  ASSERT_TRUE(ArrayEQ<IdType>(g_coo.row, r_g_coo.col));
  ASSERT_TRUE(ArrayEQ<IdType>(g_coo.col, r_g_coo.row));

  // test out coo
  hg = std::dynamic_pointer_cast<HeteroGraph>(CreateFromCOO(2, coo, SparseFormat::kAny));
  g = hg->relation_graphs()[0];
  ASSERT_EQ(g->GetFormatInUse(), 1);
  r_g = g->Reverse();
  ASSERT_EQ(r_g->GetFormatInUse(), 1);
  g_coo = g->GetCOOMatrix(0);
  r_g_coo = r_g->GetCOOMatrix(0);
  ASSERT_EQ(g_coo.num_rows, r_g_coo.num_cols);
  ASSERT_EQ(g_coo.num_cols, r_g_coo.num_rows);
  ASSERT_TRUE(g_coo.row->data == r_g_coo.col->data);
  ASSERT_TRUE(g_coo.col->data == r_g_coo.row->data);
  g_in_csr = g->GetCSCMatrix(0);
  ASSERT_EQ(g->GetFormatInUse(), 5);
  ASSERT_EQ(r_g->GetFormatInUse(), 3);
  r_g_out_csr = r_g->GetCSRMatrix(0);
  ASSERT_TRUE(g_in_csr.indptr->data == r_g_out_csr.indptr->data);
  ASSERT_TRUE(g_in_csr.indices->data == r_g_out_csr.indices->data);
  g_out_csr = g->GetCSRMatrix(0);
  ASSERT_EQ(g->GetFormatInUse(), 7);
  ASSERT_EQ(r_g->GetFormatInUse(), 7);
  r_g_in_csr = r_g->GetCSCMatrix(0);
  ASSERT_TRUE(g_out_csr.indptr->data == r_g_in_csr.indptr->data);
  ASSERT_TRUE(g_out_csr.indices->data == r_g_in_csr.indices->data);
}

TEST(UniGraphTest, TestUnitGraph_Create) {
  _TestUnitGraph<int32_t>(CPU);
  _TestUnitGraph<int64_t>(CPU);
#ifdef DGL_USE_CUDA
  _TestUnitGraph<int32_t>(GPU);
  _TestUnitGraph<int64_t>(GPU);
#endif
}

TEST(UniGraphTest, TestUnitGraph_GetInCSR) {
  _TestUnitGraph_GetInCSR<int32_t>(CPU);
  _TestUnitGraph_GetInCSR<int64_t>(CPU);
#ifdef DGL_USE_CUDA
  _TestUnitGraph_GetInCSR<int32_t>(GPU);
  _TestUnitGraph_GetInCSR<int64_t>(GPU);
#endif
}

TEST(UniGraphTest, TestUnitGraph_GetOutCSR) {
  _TestUnitGraph_GetOutCSR<int32_t>(CPU);
  _TestUnitGraph_GetOutCSR<int64_t>(CPU);
#ifdef DGL_USE_CUDA
  _TestUnitGraph_GetOutCSR<int32_t>(GPU);
  _TestUnitGraph_GetOutCSR<int64_t>(GPU);
#endif
}

TEST(UniGraphTest, TestUnitGraph_GetCOO) {
  _TestUnitGraph_GetCOO<int32_t>(CPU);
  _TestUnitGraph_GetCOO<int64_t>(CPU);
#ifdef DGL_USE_CUDA
  _TestUnitGraph_GetCOO<int32_t>(GPU);
  _TestUnitGraph_GetCOO<int64_t>(GPU);
#endif
}

TEST(UniGraphTest, TestUnitGraph_Reserve) {
  _TestUnitGraph_Reserve<int32_t>(CPU);
  _TestUnitGraph_Reserve<int64_t>(CPU);
#ifdef DGL_USE_CUDA
  _TestUnitGraph_Reserve<int32_t>(GPU);
  _TestUnitGraph_Reserve<int64_t>(GPU);
#endif
}
