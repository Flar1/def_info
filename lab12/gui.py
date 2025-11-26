"""
Графический интерфейс для игры в ментальный покер.
"""

import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
from mental_poker_game import MentalPokerGame


class MentalPokerGUI:
    """Графический интерфейс для ментального покера."""
    
    def __init__(self, root):
        self.root = root
        self.root.title("Ментальный покер - Техасский холдем")
        self.root.geometry("1200x800")
        
        self.game = None
        self.current_player_view = 0
        
        self.create_widgets()
    
    def create_widgets(self):
        """Создание виджетов интерфейса."""
        # Верхняя панель с настройками
        top_frame = ttk.Frame(self.root, padding="10")
        top_frame.pack(fill=tk.X)
        
        ttk.Label(top_frame, text="Количество игроков:").pack(side=tk.LEFT, padx=5)
        self.num_players_var = tk.StringVar(value="4")
        num_players_spin = ttk.Spinbox(top_frame, from_=2, to=10, textvariable=self.num_players_var, width=5)
        num_players_spin.pack(side=tk.LEFT, padx=5)
        
        ttk.Button(top_frame, text="Начать игру", command=self.start_game).pack(side=tk.LEFT, padx=10)
        ttk.Button(top_frame, text="Раздать карты", command=self.deal_cards).pack(side=tk.LEFT, padx=5)
        ttk.Button(top_frame, text="Флоп", command=self.deal_flop).pack(side=tk.LEFT, padx=5)
        ttk.Button(top_frame, text="Терн", command=self.deal_turn).pack(side=tk.LEFT, padx=5)
        ttk.Button(top_frame, text="Ривер", command=self.deal_river).pack(side=tk.LEFT, padx=5)
        
        # Выбор игрока для просмотра
        ttk.Label(top_frame, text="Просмотр игрока:").pack(side=tk.LEFT, padx=10)
        self.player_view_var = tk.StringVar(value="0")
        player_view_combo = ttk.Combobox(top_frame, textvariable=self.player_view_var, 
                                         values=[str(i) for i in range(10)], width=3, state="readonly")
        player_view_combo.pack(side=tk.LEFT, padx=5)
        player_view_combo.bind("<<ComboboxSelected>>", self.on_player_view_change)
        
        # Основная область с информацией об игре
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Левая панель - информация об игроках
        left_frame = ttk.LabelFrame(main_frame, text="Игроки", padding="10")
        left_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5)
        
        self.players_text = tk.Text(left_frame, height=20, wrap=tk.WORD, font=("Courier", 10))
        self.players_text.pack(fill=tk.BOTH, expand=True)
        players_scroll = ttk.Scrollbar(left_frame, orient=tk.VERTICAL, command=self.players_text.yview)
        players_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.players_text.config(yscrollcommand=players_scroll.set)
        
        # Правая панель - карты и состояние игры
        right_frame = ttk.Frame(main_frame)
        right_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=5)
        
        # Общие карты
        community_frame = ttk.LabelFrame(right_frame, text="Общие карты на столе", padding="10")
        community_frame.pack(fill=tk.X, pady=5)
        
        self.community_cards_text = tk.Text(community_frame, height=3, wrap=tk.WORD, 
                                            font=("Courier", 12, "bold"))
        self.community_cards_text.pack(fill=tk.X)
        
        # Карты текущего игрока
        hand_frame = ttk.LabelFrame(right_frame, text="Ваши карты", padding="10")
        hand_frame.pack(fill=tk.X, pady=5)
        
        self.hand_text = tk.Text(hand_frame, height=3, wrap=tk.WORD, 
                                font=("Courier", 14, "bold"), bg="#e8f5e9")
        self.hand_text.pack(fill=tk.X)
        
        # Информация о фазе игры
        phase_frame = ttk.LabelFrame(right_frame, text="Фаза игры", padding="10")
        phase_frame.pack(fill=tk.X, pady=5)
        
        self.phase_text = tk.Text(phase_frame, height=2, wrap=tk.WORD, font=("Arial", 11))
        self.phase_text.pack(fill=tk.X)
        
        # Лог действий
        log_frame = ttk.LabelFrame(right_frame, text="Лог действий", padding="10")
        log_frame.pack(fill=tk.BOTH, expand=True, pady=5)
        
        self.log_text = tk.Text(log_frame, height=10, wrap=tk.WORD, font=("Courier", 9))
        self.log_text.pack(fill=tk.BOTH, expand=True)
        log_scroll = ttk.Scrollbar(log_frame, orient=tk.VERTICAL, command=self.log_text.yview)
        log_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.log_text.config(yscrollcommand=log_scroll.set)
        
        # Нижняя панель с действиями
        bottom_frame = ttk.Frame(self.root, padding="10")
        bottom_frame.pack(fill=tk.X)
        
        ttk.Button(bottom_frame, text="Обновить вид", command=self.update_display).pack(side=tk.LEFT, padx=5)
        ttk.Button(bottom_frame, text="Сбросить карты (Fold)", command=self.fold_current_player).pack(side=tk.LEFT, padx=5)
        ttk.Button(bottom_frame, text="Сделать ставку", command=self.place_bet_dialog).pack(side=tk.LEFT, padx=5)
        ttk.Button(top_frame, text="Показать все карты", command=self.show_all_cards).pack(side=tk.LEFT, padx=10)
    
    def log(self, message):
        """Добавление сообщения в лог."""
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.see(tk.END)
    
    def start_game(self):
        """Начало новой игры."""
        try:
            num_players = int(self.num_players_var.get())
            if num_players < 2 or num_players > 10:
                messagebox.showerror("Ошибка", "Количество игроков должно быть от 2 до 10")
                return
            
            self.game = MentalPokerGame(num_players)
            self.game.setup_game()
            self.current_player_view = 0
            self.player_view_var.set("0")
            
            # Обновление списка игроков для выбора
            player_view_combo = None
            for widget in self.root.winfo_children():
                for child in widget.winfo_children():
                    if isinstance(child, ttk.Combobox):
                        child['values'] = [str(i) for i in range(num_players)]
            
            self.log(f"Игра начата с {num_players} игроками")
            self.log("Колода создана и зашифрована")
            self.update_display()
            messagebox.showinfo("Игра начата", f"Игра начата с {num_players} игроками.\nКолода зашифрована и перемешана.")
        except Exception as e:
            messagebox.showerror("Ошибка", f"Не удалось начать игру: {str(e)}")
            self.log(f"ОШИБКА: {str(e)}")
    
    def deal_cards(self):
        """Раздача карт игрокам."""
        if not self.game:
            messagebox.showwarning("Предупреждение", "Сначала начните игру")
            return
        
        try:
            self.game.deal_player_cards()
            self.log("Карты разданы игрокам")
            self.update_display()
            messagebox.showinfo("Карты разданы", "Каждому игроку раздано по 2 карты")
        except Exception as e:
            messagebox.showerror("Ошибка", f"Не удалось раздать карты: {str(e)}")
            self.log(f"ОШИБКА: {str(e)}")
    
    def deal_flop(self):
        """Раздача флопа."""
        if not self.game:
            messagebox.showwarning("Предупреждение", "Сначала начните игру и раздайте карты")
            return
        
        try:
            self.game.deal_flop()
            self.log("Раздан флоп (3 общие карты)")
            self.update_display()
        except Exception as e:
            messagebox.showerror("Ошибка", f"Не удалось раздать флоп: {str(e)}")
            self.log(f"ОШИБКА: {str(e)}")
    
    def deal_turn(self):
        """Раздача терна."""
        if not self.game:
            messagebox.showwarning("Предупреждение", "Сначала раздайте флоп")
            return
        
        try:
            self.game.deal_turn()
            self.log("Раздан терн (4-я общая карта)")
            self.update_display()
        except Exception as e:
            messagebox.showerror("Ошибка", f"Не удалось раздать терн: {str(e)}")
            self.log(f"ОШИБКА: {str(e)}")
    
    def deal_river(self):
        """Раздача ривера."""
        if not self.game:
            messagebox.showwarning("Предупреждение", "Сначала раздайте терн")
            return
        
        try:
            self.game.deal_river()
            self.log("Раздан ривер (5-я общая карта)")
            self.update_display()
        except Exception as e:
            messagebox.showerror("Ошибка", f"Не удалось раздать ривер: {str(e)}")
            self.log(f"ОШИБКА: {str(e)}")
    
    def on_player_view_change(self, event=None):
        """Изменение выбранного игрока для просмотра."""
        try:
            self.current_player_view = int(self.player_view_var.get())
            self.update_display()
        except:
            pass
    
    def update_display(self):
        """Обновление отображения игры."""
        if not self.game:
            return
        
        # Очистка текстовых полей
        self.players_text.delete(1.0, tk.END)
        self.community_cards_text.delete(1.0, tk.END)
        self.hand_text.delete(1.0, tk.END)
        self.phase_text.delete(1.0, tk.END)
        
        # Информация об игроках
        state = self.game.get_game_state()
        players_info = []
        for player in state['players']:
            status = "FOLDED" if player['folded'] else "ACTIVE"
            hand_str = ", ".join(player['hand']) if player['hand'] else "Скрыто"
            players_info.append(
                f"Игрок {player['id']}: {status}\n"
                f"  Карты: {hand_str}\n"
                f"  Фишки: {player['chips']}, Ставка: {player['bet']}\n"
            )
        self.players_text.insert(1.0, "\n".join(players_info))
        
        # Общие карты
        if state['community_cards']:
            self.community_cards_text.insert(1.0, ", ".join(state['community_cards']))
        else:
            self.community_cards_text.insert(1.0, "Карты еще не разданы")
        
        # Карты выбранного игрока
        if self.current_player_view < len(state['players']):
            player = state['players'][self.current_player_view]
            if player['hand']:
                self.hand_text.insert(1.0, ", ".join(player['hand']))
            else:
                self.hand_text.insert(1.0, "Карты еще не разданы")
        
        # Фаза игры
        phase_names = {
            'setup': 'Настройка',
            'preflop': 'Префлоп (до флопа)',
            'flop': 'Флоп (3 общие карты)',
            'turn': 'Терн (4 общие карты)',
            'river': 'Ривер (5 общих карт)',
            'showdown': 'Вскрытие карт'
        }
        phase_name = phase_names.get(state['phase'], state['phase'])
        self.phase_text.insert(1.0, f"Текущая фаза: {phase_name}")
    
    def fold_current_player(self):
        """Текущий игрок сбрасывает карты."""
        if not self.game:
            messagebox.showwarning("Предупреждение", "Сначала начните игру")
            return
        
        try:
            self.game.fold_player(self.current_player_view)
            self.log(f"Игрок {self.current_player_view} сбросил карты")
            self.update_display()
        except Exception as e:
            messagebox.showerror("Ошибка", f"Ошибка: {str(e)}")
    
    def place_bet_dialog(self):
        """Диалог для размещения ставки."""
        if not self.game:
            messagebox.showwarning("Предупреждение", "Сначала начните игру")
            return
        
        try:
            amount = simpledialog.askinteger("Ставка", "Введите размер ставки:", minvalue=1)
            if amount:
                if self.game.place_bet(self.current_player_view, amount):
                    self.log(f"Игрок {self.current_player_view} поставил {amount}")
                    self.update_display()
                else:
                    messagebox.showerror("Ошибка", "Недостаточно фишек")
        except Exception as e:
            messagebox.showerror("Ошибка", f"Ошибка: {str(e)}")
    
    def show_all_cards(self):
        """Показать все карты всех игроков (для демонстрации)."""
        if not self.game:
            messagebox.showwarning("Предупреждение", "Сначала начните игру")
            return
        
        state = self.game.get_game_state()
        info = "=== ВСЕ КАРТЫ ===\n\n"
        for player in state['players']:
            info += f"Игрок {player['id']}: {', '.join(player['hand'])}\n"
        info += f"\nОбщие карты: {', '.join(state['community_cards'])}\n"
        
        messagebox.showinfo("Все карты", info)


def main():
    """Главная функция для запуска приложения."""
    root = tk.Tk()
    app = MentalPokerGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()

