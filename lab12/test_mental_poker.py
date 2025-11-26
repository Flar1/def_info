"""
Тестовый скрипт для проверки работы ментального покера.
"""

from mental_poker_game import MentalPokerGame


def test_basic_game():
    """Базовый тест игры."""
    print("=== Тест ментального покера ===\n")
    
    # Создание игры с 3 игроками
    num_players = 3
    game = MentalPokerGame(num_players)
    print(f"Создана игра с {num_players} игроками")
    
    # Настройка игры
    game.setup_game()
    print("Колода создана и зашифрована")
    print(f"Размер зашифрованной колоды: {len(game.encrypted_deck)} карт\n")
    
    # Раздача карт
    game.deal_player_cards()
    print("Карты разданы игрокам")
    
    # Показ карт каждого игрока
    for i in range(num_players):
        hand = game.get_player_hand(i)
        print(f"Игрок {i}: {hand}")
    
    print("\nРаздача флопа...")
    game.deal_flop()
    print(f"Общие карты (флоп): {game.get_community_cards()}")
    
    print("\nРаздача терна...")
    game.deal_turn()
    print(f"Общие карты (терн): {game.get_community_cards()}")
    
    print("\nРаздача ривера...")
    game.deal_river()
    print(f"Общие карты (ривер): {game.get_community_cards()}")
    
    print("\n=== Тест завершен успешно ===")


if __name__ == "__main__":
    try:
        test_basic_game()
    except Exception as e:
        print(f"ОШИБКА: {str(e)}")
        import traceback
        traceback.print_exc()

