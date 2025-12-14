import random
import sys

class HamiltonianGraphGenerator:
    def __init__(self):
        self.n = 0
        self.edges = []
        self.adj = []
        self.cycle = []
        
    def add_edge(self, u, v):
        if u == v:
            return
        if v not in self.adj[u]:
            self.adj[u].add(v)
            self.adj[v].add(u)
            self.edges.append((u, v))

    def generate_random_graph(self, n, m):
        self.n = n
        self.adj = [set() for _ in range(n)]
        self.edges = []

        self.cycle = list(range(n))
        random.shuffle(self.cycle)

        for i in range(n):
            u = self.cycle[i]
            v = self.cycle[(i + 1) % n]
            self.add_edge(u, v)

        attempts = 0
        while len(self.edges) < m and attempts < m * 10:
            u = random.randint(0, n - 1)
            v = random.randint(0, n - 1)
            self.add_edge(u, v)
            attempts += 1

    def is_valid_cycle(self):
        for i in range(self.n):
            u = self.cycle[i]
            v = self.cycle[(i + 1) % self.n]
            if v not in self.adj[u]:
                return False
        return True

    def generate(self, n, m):
        if n < 3:
            print("Ошибка: нужно минимум 3 вершины")
            return False

        max_edges = n * (n - 1) // 2
        if m > max_edges:
            print(f"Слишком много ребер! Максимум: {max_edges}")
            return False

        print("Генерация графа с гамильтоновым циклом...")
        self.generate_random_graph(n, m)

        print("Проверка корректности цикла...")
        if not self.is_valid_cycle():
            print("Ошибка: цикл некорректен (что маловероятно).")
            return False

        print("Граф успешно создан.")
        print(f"Вершины: {self.n}")
        print(f"Рёбра: {len(self.edges)}")
        print(f"Гамильтонов цикл: {self.cycle}")

        return True

    def save_to_file(self, filename):
        try:
            with open(filename, "w") as f:
                f.write(f"{self.n} {len(self.edges)}\n")
                for u, v in self.edges:
                    f.write(f"{u} {v}\n")

                f.write(" ".join(map(str, self.cycle)) + "\n")

            print(f"Граф сохранён в файл {filename}")
            return True

        except Exception as e:
            print("Ошибка сохранения:", e)
            return False


def main():
    print("=== Генератор графов с гамильтоновым циклом (Python) ===")

    n = int(input("Введите количество вершин: "))
    m = int(input("Введите количество рёбер: "))

    generator = HamiltonianGraphGenerator()

    if not generator.generate(n, m):
        sys.exit(1)

    filename = input("Введите имя выходного файла: ")
    if not generator.save_to_file(filename):
        sys.exit(1)

    print("✓ Готово! Теперь можно использовать этот граф в протоколе ZKP.")


if __name__ == "__main__":
    main()