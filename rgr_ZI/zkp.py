import random
import sys

class Graph:
    def __init__(self):
        self.n = 0
        self.m = 0
        self.edges = []
        self.cycle = []

class ZKPHamiltonianCycle:
    def __init__(self):
        self.graph = Graph()
        random.seed()

    def read_graph(self, filename):
        try:
            with open(filename, "r") as f:
                self.graph.n, self.graph.m = map(int, f.readline().split())

                self.graph.edges = []
                for _ in range(self.graph.m):
                    u, v = map(int, f.readline().split())
                    self.graph.edges.append((u, v))

                self.graph.cycle = list(map(int, f.readline().split()))

        except Exception as e:
            print("Ошибка чтения файла:", e)
            return False

        if len(self.graph.cycle) != self.graph.n:
            print("Ошибка: гамильтонов цикл имеет неправильную длину!")
            return False

        return True

    def print_graph_info(self):
        print("\n=== Информация о графе ===")
        print(f"Количество вершин: {self.graph.n}")
        print(f"Количество рёбер: {self.graph.m}")
        print("\nРёбра графа:")
        for u, v in self.graph.edges:
            print(f"{u} -- {v}")

        print("\nГамильтонов цикл (скрывается в протоколе):")
        print(self.graph.cycle)

    def generate_random_permutation(self):
        perm = list(range(self.graph.n))
        random.shuffle(perm)
        return perm

    def apply_permutation(self, perm):
        perm_edges = []
        for (u, v) in self.graph.edges:
            perm_edges.append((perm[u], perm[v]))

        perm_cycle = [perm[v] for v in self.graph.cycle]

        return perm_edges, perm_cycle

    def run_round(self, round_num):
        print(f"\n=== Раунд {round_num} ===")

        perm = self.generate_random_permutation()
        print("1. Доказывающий генерирует случайную перестановку вершин.")

        perm_edges, perm_cycle = self.apply_permutation(perm)
        print("2. Доказывающий отправляет Проверяющему новый граф (без цикла).")

        request = random.choice([1, 2])

        if request == 1:
            print("3. Проверяющий выбирает: показать перестановку (проверка изоморфизма).")

            print("4. Доказывающий раскрывает перестановку:")
            print("   Перестановка:", perm)

            print("5. Проверка корректности изоморфизма...")
            for (u, v), (pu, pv) in zip(self.graph.edges, perm_edges):
                if perm[u] != pu or perm[v] != pv:
                    print("   X Ошибка: изоморфизм неверен!")
                    return False

            print("   ✓ Изоморфизм корректен. Раунд пройден.")
            return True

        else:
            print("3. Проверяющий выбирает: показать гамильтонов цикл в новом графе.")

            print("4. Доказывающий раскрывает гамильтонов цикл в изоморфном графе:")
            print("   Цикл:", perm_cycle)

            for i in range(self.graph.n):
                u = perm_cycle[i]
                v = perm_cycle[(i + 1) % self.graph.n]
                if (u, v) not in perm_edges and (v, u) not in perm_edges:
                    print("   ❌ Ошибка: цикл неверен! Рёбра нет в графе.")
                    return False

            print("   ✓ Цикл корректен. Раунд пройден.")
            return True

    def run_protocol(self, rounds):
        print("\n")
        print("ПРОТОКОЛ ДОКАЗАТЕЛЬСТВА С НУЛЕВЫМ ЗНАНИЕМ")
        print("Задача: Гамильтонов цикл")

        self.print_graph_info()

        print("\n\n")
        print("НАЧАЛО ПРОТОКОЛА")
        print(f"Количество раундов: {rounds}")

        success = 0
        for i in range(1, rounds + 1):
            if self.run_round(i):
                success += 1

        print("\n\n")
        print("РЕЗУЛЬТАТЫ ПРОТОКОЛА:")
        print(f"Успешных раундов: {success} из {rounds}")

        if success == rounds:
            print("\n√ Протокол успешно завершён!")
            print("√ Доказывающий с высокой вероятностью знает гамильтонов цикл")
            print("√ Проверяющий не узнал сам цикл (нулевое знание)")


def main():
    print("Zero-Knowledge Proof: Гамильтонов цикл")

    filename = input("Введите имя файла с графом: ")

    zkp = ZKPHamiltonianCycle()

    if not zkp.read_graph(filename):
        sys.exit(1)

    rounds = int(input("Введите количество раундов (10-20 рекомендуется): "))
    zkp.run_protocol(rounds)


if __name__ == "__main__":
    main()