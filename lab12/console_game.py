"""
Консольная версия игры в ментальный покер (Техасский холдем).
Работает без графического интерфейса.
"""

from mental_poker_game import MentalPokerGame


class ConsoleMentalPoker:
    """Консольный интерфейс для ментального покера."""
    
    def __init__(self):
        self.game = None
        self.num_players = 0
    
    def print_separator(self):
        """Печать разделителя."""
        print("\n" + "="*60 + "\n")
    
    def print_menu(self):
        """Печать меню."""
        print("\n=== МЕНЮ ===")
        print("1. Начать новую игру")
        print("2. Раздать карты игрокам")
        print("3. Раздать флоп (3 общие карты)")
        print("4. Раздать терн (4-я общая карта)")
        print("5. Раздать ривер (5-я общая карта)")
        print("6. Показать состояние игры")
        print("7. Показать карты игрока")
        print("8. Игрок сбрасывает карты (Fold)")
        print("9. Игрок делает ставку")
        print("0. Выход")
        print("="*60)
    
    def start_game(self):
        """Начало новой игры."""
        try:
            num_players = input("Введите количество игроков (2-10): ").strip()
            num_players = int(num_players)
            if num_players < 2 or num_players > 10:
                print("Ошибка: количество игроков должно быть от 2 до 10")
                return
            
            self.game = MentalPokerGame(num_players)
            self.game.setup_game()
            self.num_players = num_players
            print(f"\n✓ Игра начата с {num_players} игроками")
            print("✓ Колода создана и зашифрована")
            print(f"✓ Размер колоды: {len(self.game.encrypted_deck)} карт")
        except ValueError:
            print("Ошибка: введите корректное число")
        except Exception as e:
            print(f"Ошибка: {str(e)}")
    
    def deal_cards(self):
        """Раздача карт."""
        if not self.game:
            print("Ошибка: сначала начните игру")
            return
        
        try:
            self.game.deal_player_cards()
            print("\n✓ Карты разданы игрокам")
            self.show_player_hands()
        except Exception as e:
            print(f"Ошибка: {str(e)}")
    
    def deal_flop(self):
        """Раздача флопа."""
        if not self.game:
            print("Ошибка: сначала начните игру и раздайте карты")
            return
        
        try:
            self.game.deal_flop()
            print("\n✓ Раздан флоп (3 общие карты)")
            self.show_community_cards()
        except Exception as e:
            print(f"Ошибка: {str(e)}")
    
    def deal_turn(self):
        """Раздача терна."""
        if not self.game:
            print("Ошибка: сначала раздайте флоп")
            return
        
        try:
            self.game.deal_turn()
            print("\n✓ Раздан терн (4-я общая карта)")
            self.show_community_cards()
        except Exception as e:
            print(f"Ошибка: {str(e)}")
    
    def deal_river(self):
        """Раздача ривера."""
        if not self.game:
            print("Ошибка: сначала раздайте терн")
            return
        
        try:
            self.game.deal_river()
            print("\n✓ Раздан ривер (5-я общая карта)")
            self.show_community_cards()
        except Exception as e:
            print(f"Ошибка: {str(e)}")
    
    def show_game_state(self):
        """Показать состояние игры."""
        if not self.game:
            print("Ошибка: игра не начата")
            return
        
        state = self.game.get_game_state()
        phase_names = {
            'setup': 'Настройка',
            'preflop': 'Префлоп',
            'flop': 'Флоп',
            'turn': 'Терн',
            'river': 'Ривер',
            'showdown': 'Вскрытие'
        }
        
        self.print_separator()
        print(f"Фаза игры: {phase_names.get(state['phase'], state['phase'])}")
        print(f"Общие карты: {', '.join(state['community_cards']) if state['community_cards'] else 'Нет'}")
        print("\nИгроки:")
        for player in state['players']:
            status = "FOLDED" if player['folded'] else "ACTIVE"
            hand_str = ", ".join(player['hand']) if player['hand'] else "Скрыто"
            print(f"  Игрок {player['id']}: {status}")
            print(f"    Карты: {hand_str}")
            print(f"    Фишки: {player['chips']}, Ставка: {player['bet']}")
        self.print_separator()
    
    def show_player_hands(self):
        """Показать карты всех игроков."""
        if not self.game:
            print("Ошибка: игра не начата")
            return
        
        state = self.game.get_game_state()
        print("\nКарты игроков:")
        for player in state['players']:
            hand_str = ", ".join(player['hand']) if player['hand'] else "Скрыто"
            print(f"  Игрок {player['id']}: {hand_str}")
    
    def show_player_hand(self):
        """Показать карты конкретного игрока."""
        if not self.game:
            print("Ошибка: игра не начата")
            return
        
        try:
            player_id = input("Введите ID игрока (0-{}): ".format(self.num_players - 1)).strip()
            player_id = int(player_id)
            if player_id < 0 or player_id >= self.num_players:
                print("Ошибка: неверный ID игрока")
                return
            
            hand = self.game.get_player_hand(player_id)
            print(f"\nКарты игрока {player_id}: {', '.join(hand) if hand else 'Скрыто'}")
        except ValueError:
            print("Ошибка: введите корректное число")
        except Exception as e:
            print(f"Ошибка: {str(e)}")
    
    def fold_player(self):
        """Игрок сбрасывает карты."""
        if not self.game:
            print("Ошибка: игра не начата")
            return
        
        try:
            player_id = input("Введите ID игрока для сброса карт: ").strip()
            player_id = int(player_id)
            if player_id < 0 or player_id >= self.num_players:
                print("Ошибка: неверный ID игрока")
                return
            
            self.game.fold_player(player_id)
            print(f"\n✓ Игрок {player_id} сбросил карты")
        except ValueError:
            print("Ошибка: введите корректное число")
        except Exception as e:
            print(f"Ошибка: {str(e)}")
    
    def place_bet(self):
        """Игрок делает ставку."""
        if not self.game:
            print("Ошибка: игра не начата")
            return
        
        try:
            player_id = input("Введите ID игрока: ").strip()
            player_id = int(player_id)
            if player_id < 0 or player_id >= self.num_players:
                print("Ошибка: неверный ID игрока")
                return
            
            amount = input("Введите размер ставки: ").strip()
            amount = int(amount)
            
            if self.game.place_bet(player_id, amount):
                print(f"\n✓ Игрок {player_id} поставил {amount}")
            else:
                print("Ошибка: недостаточно фишек")
        except ValueError:
            print("Ошибка: введите корректное число")
        except Exception as e:
            print(f"Ошибка: {str(e)}")
    
    def show_community_cards(self):
        """Показать общие карты."""
        if not self.game:
            return
        
        cards = self.game.get_community_cards()
        if cards:
            print(f"Общие карты: {', '.join(cards)}")
        else:
            print("Общие карты еще не разданы")
    
    def run(self):
        """Запуск консольной версии."""
        print("\n" + "="*60)
        print("  МЕНТАЛЬНЫЙ ПОКЕР - ТЕХАССКИЙ ХОЛДЕМ")
        print("="*60)
        
        while True:
            self.print_menu()
            choice = input("\nВыберите действие: ").strip()
            
            if choice == '1':
                self.start_game()
            elif choice == '2':
                self.deal_cards()
            elif choice == '3':
                self.deal_flop()
            elif choice == '4':
                self.deal_turn()
            elif choice == '5':
                self.deal_river()
            elif choice == '6':
                self.show_game_state()
            elif choice == '7':
                self.show_player_hand()
            elif choice == '8':
                self.fold_player()
            elif choice == '9':
                self.place_bet()
            elif choice == '0':
                print("\nДо свидания!")
                break
            else:
                print("Неверный выбор. Попробуйте снова.")


def main():
    """Главная функция."""
    game = ConsoleMentalPoker()
    game.run()


if __name__ == "__main__":
    main()

