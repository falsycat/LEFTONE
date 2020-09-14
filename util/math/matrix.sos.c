#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SWAP(a, b) do {  \
  typeof(a) macro_SWAP_temp_ = a;  \
  a = b;  \
  b = macro_SWAP_temp_;  \
} while (0)

static void next_permutation(size_t* p, size_t* c, size_t len) {
  assert(p != NULL);
  assert(c != NULL);

  size_t i = 0;
  while (i < len) {
    if (c[i] < i) {
      if (i%2 == 0) {
        SWAP(p[0], p[i]);
      } else {
        SWAP(p[c[i]], p[i]);
      }
      ++c[i];
      return;
    }
    c[i] = 0;
    ++i;
  }
}

static void valid(size_t dim) {
  assert(dim >= 2);

  printf("bool mat%zu_valid(const mat%zu_t* m) {", dim, dim);

  printf("return m != NULL && MATH_FLOAT_VALID(m->ptr[0])");
  for (size_t i = 1; i < dim*dim; ++i) {
    printf("&& MATH_FLOAT_VALID(m->ptr[%zu])", i);
  }
  printf(";}\n");
}

static void each(size_t dim, const char* name, char op) {
  assert(dim >= 2);

  printf("void mat%zu_%s(mat%zu_t* x, const mat%zu_t* l, const mat%zu_t* r) {", dim, name, dim, dim, dim);

  printf("assert(x != NULL);");
  printf("assert(mat%zu_valid(l));", dim);
  printf("assert(mat%zu_valid(r));", dim);

  for (size_t i = 0; i < dim*dim; ++i) {
    printf("x->ptr[%zu] = l->ptr[%zu] %c r->ptr[%zu];", i, i, op, i);
  }
  printf("}\n");
}

static void eacheq(size_t dim, const char* name, char op) {
  assert(dim >= 2);

  printf("void mat%zu_%s(mat%zu_t* x, const mat%zu_t* r) {", dim, name, dim, dim);

  printf("assert(mat%zu_valid(x));", dim);
  printf("assert(mat%zu_valid(r));", dim);

  for (size_t i = 0; i < dim*dim; ++i) {
    printf("x->ptr[%zu] %c= r->ptr[%zu];", i, op, i);
  }
  printf("}\n");
}

static void mul(size_t dim) {
  assert(dim >= 2);

  printf("void mat%zu_mul(mat%zu_t* x, const mat%zu_t* l, const mat%zu_t* r) {", dim, dim, dim, dim);

  printf("assert(x != NULL);");
  printf("assert(mat%zu_valid(l));", dim);
  printf("assert(mat%zu_valid(r));", dim);

  for (size_t i = 0; i < dim; ++i) {
    for (size_t j = 0; j < dim; ++j) {
      printf("x->elm[%zu][%zu] = 0", j, i);
      for (size_t k = 0; k < dim; ++k) {
        printf("+ l->elm[%zu][%zu] * r->elm[%zu][%zu]", k, i, j, k);
      }
      printf(";");
    }
  }
  printf("}\n");
}

static void mul_vec(size_t dim) {
  assert(dim >= 2);

  printf("void mat%zu_mul_vec%zu(vec%zu_t* x, const mat%zu_t* l, const vec%zu_t* r) {", dim, dim, dim, dim, dim);

  printf("assert(x != NULL);");
  printf("assert(mat%zu_valid(l));", dim);
  printf("assert(vec%zu_valid(r));", dim);

  for (size_t i = 0; i < dim; ++i) {
    printf("x->ptr[%zu] = 0", i);
    for (size_t j = 0; j < dim; ++j) {
      printf("+ l->elm[%zu][%zu] * r->ptr[%zu]", j, i, j);
    }
    printf(";");
  }
  printf("}\n");
}

static void det(size_t dim) {
  assert(dim >= 2);

  size_t p[dim];
  for (size_t i = 0; i < dim; ++i) p[i] = i;

  size_t c[dim];
  for (size_t i = 0; i < dim; ++i) c[i] = 0;

  printf("float mat%zu_det(const mat%zu_t* x) {", dim, dim);
  printf("assert(mat%zu_valid(x));", dim);

  size_t f = 1;
  for (size_t i = 1; i <= dim; ++i) f *= i;

  printf("return 0");

  bool positive = true;
  for (size_t i = 0; i < f; ++i) {
    printf("%c1", positive? '+': '-');
    for (size_t j = 0; j < dim; ++j) {
      printf("*x->elm[%zu][%zu]", p[j], j);
    }
    next_permutation(p, c, dim);
    positive = !positive;
  }
  printf(";}\n");
}

static void cofactor(size_t dim) {
  assert(dim >= 3);

  printf("void mat%zu_cofactor(mat%zu_t* x, const mat%zu_t* r, size_t row, size_t col) {", dim, dim-1, dim);

  printf("assert(x != NULL);");
  printf("assert(mat%zu_valid(r));", dim);
  printf("assert(row < %zu);", dim);
  printf("assert(col < %zu);", dim);

  for (size_t row = 0; row < dim; ++row) {
    for (size_t col = 0; col < dim; ++col) {
      printf("if (row == %zu && col == %zu) {", row, col);
      for (size_t xi = 0; xi < dim-1; ++xi) {
        const size_t ri = xi >= row? xi+1: xi;
        for (size_t xj = 0; xj < dim-1; ++xj) {
          const size_t rj = xj >= col? xj+1: xj;
          printf("x->elm[%zu][%zu] = %s r->elm[%zu][%zu];", xj, xi, (xj+xi)%2 == 0? "": "-", rj, ri);
        }
      }
      printf("return;}");
    }
  }
  printf("}\n");
}

static void inv(size_t dim) {
  assert(dim >= 3);

  printf("bool mat%zu_inv(mat%zu_t* x, const mat%zu_t* r) {", dim, dim, dim);

  printf("assert(x != NULL);");
  printf("assert(mat%zu_valid(r));", dim);

  printf("const float d = mat%zu_det(r);", dim);
  printf("if (d == 0) return false;");

  printf("mat%zu_t co;", dim-1);
  for (size_t i = 0; i < dim; ++i) {
    for (size_t j = 0; j < dim; ++j) {
      printf("mat%zu_cofactor(&co, r, %zu, %zu);", dim, i, j);
      printf("x->elm[%zu][%zu] = mat%zu_det(&co) / d;", j, i, dim-1);
    }
  }
  printf("return true;");
  printf("}\n");
}

int main(int argc, char** argv) {
  (void) argc, (void) argv;

  printf("#include \"./matrix.h\"\n");

  printf("#include <assert.h>\n");
  printf("#include <stdbool.h>\n");
  printf("#include <stddef.h>\n");

  printf("#include \"./algorithm.h\"\n");
  printf("#include \"./vector.h\"\n");

  valid(2);
  valid(3);
  valid(4);

  each(2, "add", '+');
  each(3, "add", '+');
  each(4, "add", '+');

  eacheq(2, "addeq", '+');
  eacheq(3, "addeq", '+');
  eacheq(4, "addeq", '+');

  each(2, "sub", '-');
  each(3, "sub", '-');
  each(4, "sub", '-');

  eacheq(2, "subeq", '-');
  eacheq(3, "subeq", '-');
  eacheq(4, "subeq", '-');

  mul(2);
  mul(3);
  mul(4);

  mul_vec(2);
  mul_vec(3);
  mul_vec(4);

  det(2);
  det(3);
  det(4);

  cofactor(3);
  cofactor(4);

  inv(3);
  inv(4);
}
