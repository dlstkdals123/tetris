#pragma once

/**
* gamestate Ŭ����
* ���� ������ ���¸� ��Ÿ���� Ŭ����
*/
class gameState
{
private:
	int level;  // ���� �÷����ϴ� ���� ����
	int score;  // ���� ȹ���� ����
	int line;   // ���� Ŭ������ ���� ��

public:
    // ������
    gameState();

    // �ʱ�ȭ �Լ� (gameover�� ���)
    void resetState();

    // Getter �޼���
    int getLevel() const;
    int getScore() const;
    int getLines() const;

    // Setter �޼���
    void setLevel(int lvl);

    // ���� ������Ʈ �޼���
    void addScore(int amount);
    void addLines(int amount);
    void levelUp();      // ������ 1 �ø��� ������ 0���� �ʱ�ȭ
    void resetLines();   // ���� ���� �ʱ�ȭ
};

