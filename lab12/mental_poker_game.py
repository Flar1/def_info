"""
Основная логика игры в ментальный покер (Техасский холдем).
"""

import random
from crypto_mental_poker import MentalPokerCrypto


class MentalPokerGame:
    """
    Класс для управления игрой в ментальный покер.
    """
    
    def __init__(self, num_players):
        """
        Инициализация игры.
        
        Args:
            num_players: Количество игроков
        """
        self.num_players = num_players
        self.players = []
        self.community_cards = []
        self.deck = []
        self.encrypted_deck = []
        self.current_player = 0
        self.game_phase = "setup"  # setup, preflop, flop, turn, river, showdown
        
        # Инициализация игроков
        for i in range(num_players):
            crypto = MentalPokerCrypto()
            self.players.append({
                'id': i,
                'crypto': crypto,
                'public_key': crypto.get_public_key(),
                'hand': [],
                'encrypted_hand': [],
                'folded': False,
                'chips': 1000,
                'bet': 0
            })
    
    def setup_game(self):
        """Настройка игры: создание и шифрование колоды."""
        # Создание колоды
        self.deck = MentalPokerCrypto.create_deck()
        
        # Получение всех публичных ключей
        public_keys = [player['public_key'] for player in self.players]
        
        # Шифрование каждой карты всеми ключами
        self.encrypted_deck = []
        for card in self.deck:
            encrypted = self.players[0]['crypto'].encrypt_card_multiple(card, public_keys)
            self.encrypted_deck.append(encrypted)
        
        # Перемешивание зашифрованной колоды
        self.encrypted_deck = MentalPokerCrypto.shuffle_encrypted_deck(self.encrypted_deck)
        
        self.game_phase = "preflop"
    
    def deal_player_cards(self):
        """Раздача карт игрокам (по 2 карты каждому)."""
        card_index = 0
        for player in self.players:
            # Раздаем 2 карты каждому игроку
            for _ in range(2):
                if card_index < len(self.encrypted_deck):
                    encrypted_card = self.encrypted_deck[card_index]
                    player['encrypted_hand'].append(encrypted_card)
                    card_index += 1
        
        # Расшифровка карт каждым игроком
        # Каждый игрок расшифровывает карту своим ключом
        for player in self.players:
            player['hand'] = []
            for encrypted_card in player['encrypted_hand']:
                try:
                    # Расшифровка карты своим ключом
                    decrypted_bytes = player['crypto'].decrypt_card_once(encrypted_card)
                    if isinstance(decrypted_bytes, bytes):
                        card = decrypted_bytes.decode('utf-8')
                    else:
                        card = str(decrypted_bytes)
                    player['hand'].append(card)
                except Exception as e:
                    # Если не удалось расшифровать, карта остается скрытой
                    player['hand'].append("HIDDEN")
        
        # Удаляем разданные карты из колоды
        cards_to_remove = self.num_players * 2
        self.encrypted_deck = self.encrypted_deck[cards_to_remove:]
    
    def deal_community_cards(self, count):
        """
        Раздача общих карт на стол.
        
        Args:
            count: Количество карт для раздачи (3 для флопа, 1 для терна и ривера)
        """
        for _ in range(count):
            if self.encrypted_deck:
                encrypted_card = self.encrypted_deck.pop(0)
                # Расшифровка общих карт
                # Пробуем расшифровать первым игроком, если не получается - другими
                card_decrypted = False
                for player in self.players:
                    try:
                        decrypted_bytes = player['crypto'].decrypt_card_once(encrypted_card)
                        if isinstance(decrypted_bytes, bytes):
                            card = decrypted_bytes.decode('utf-8')
                        else:
                            card = str(decrypted_bytes)
                        self.community_cards.append(card)
                        card_decrypted = True
                        break
                    except:
                        continue
                
                if not card_decrypted:
                    self.community_cards.append("HIDDEN")
    
    def deal_flop(self):
        """Раздача флопа (3 общие карты)."""
        self.deal_community_cards(3)
        self.game_phase = "flop"
    
    def deal_turn(self):
        """Раздача терна (4-я общая карта)."""
        self.deal_community_cards(1)
        self.game_phase = "turn"
    
    def deal_river(self):
        """Раздача ривера (5-я общая карта)."""
        self.deal_community_cards(1)
        self.game_phase = "river"
    
    def get_player_hand(self, player_id):
        """Получить карты игрока."""
        return self.players[player_id]['hand']
    
    def get_community_cards(self):
        """Получить общие карты на столе."""
        return self.community_cards
    
    def fold_player(self, player_id):
        """Игрок сбрасывает карты."""
        self.players[player_id]['folded'] = True
    
    def place_bet(self, player_id, amount):
        """Игрок делает ставку."""
        if self.players[player_id]['chips'] >= amount:
            self.players[player_id]['chips'] -= amount
            self.players[player_id]['bet'] += amount
            return True
        return False
    
    def get_game_state(self):
        """Получить текущее состояние игры."""
        return {
            'phase': self.game_phase,
            'community_cards': self.community_cards,
            'players': [
                {
                    'id': p['id'],
                    'hand': p['hand'],
                    'folded': p['folded'],
                    'chips': p['chips'],
                    'bet': p['bet']
                }
                for p in self.players
            ]
        }

