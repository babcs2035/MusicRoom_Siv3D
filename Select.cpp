// include
#include <Siv3D.hpp>
#include <vector>
#include "Select.h"
#include "Bar.h"

// define
#define WELCOME_MESSAGE_MILLISEC 3000
#define MAIN_SKEL_LIMIT 80
#define COM_MESSAGE_MILLISEC 1250

// �֐��v���g�^�C�v�錾
Rect MakeRect(int32_t x, int32_t y);
Texture SelectImage(int32_t cou);
void DrawDetails(int32_t cou);

// �A���o���\����
struct Album
{
	String name;
	String creator;
	String comment;
	Texture image;
};

// �O���[�o���萔�E�ϐ�
static Image main_tmp;
static Texture main, no_img;
static Texture fav, power;
static Texture Gaussian;
static const RectF main_rect(0, BAR_HEIGHT, 768, 768);
static int64_t startTime;
static int64_t nowTime;
static std::vector<std::pair<int64_t, int64_t>>comTime;
static int32_t skel;
static int32_t first_cou;
static bool scr_flag = true;
static Grid<double_t> z;
static TextReader reader;
static std::vector<Album> AlbumList;

// �A���o���I�� ������
void Select_Init()
{
	// ���C���w�i
	{
		main_tmp = Image(L"data\\Select\\main.png");
		main = Texture(main_tmp);
		Gaussian = Texture(main_tmp.gaussianBlurred(20, 20));
	}

	fav = Texture(L"data\\Select\\fav.png");
	power = Texture(L"data\\Select\\power.png");
	no_img = Texture(L"data\\Select\\no_img.png");

	// album_list �ǂݍ���
	{
		String temp;
		reader = TextReader(L"music\\album_list.txt");
		while (reader.readLine(temp))
		{
			String name, creator, comment;
			TextReader text(L"music\\" + temp + L"\\" + temp + L".txt");
			text.readLine(name);
			text.readLine(creator);
			String temp_of_temp;
			while (text.readLine(temp_of_temp)) { comment += temp_of_temp; }
			Texture image(L"music\\" + temp + L"\\" + temp + L".png");
			if (!image) { image = no_img; }
			AlbumList.push_back({ name,creator,comment,image });
		}
		AlbumList.push_back({ L"�V���O����",L"��Ȏ�",L"�R�����g",Texture(L"data\\Select\\single.png") });
		z = Grid<double>(3, (AlbumList.size() + 2) / 3 + 1);
	}

	startTime = Time::GetMillisec64();
	comTime.resize(AlbumList.size() + 2);
}

// �A���o���I�� �X�V
void Select_Update()
{
	skel = (skel < MAIN_SKEL_LIMIT ? skel + 1 : skel);
	nowTime = Time::GetMillisec64();

	// �X�N���[�� �X�V
	{
		scr_flag = ((first_cou + 8 <= (signed)AlbumList.size()) || (first_cou > 0) ? true : false);
		if (scr_flag)
		{
			first_cou += Mouse::Wheel() * 3;
			first_cou = Max(first_cou, 0);
			first_cou = Min<int32_t>(first_cou, AlbumList.size() / 3 * 3);
		}
	}
}

// �A���o���I�� �`��
void Select_Draw()
{
	// ���C���w�i
	{
		main.draw(0, BAR_HEIGHT);
		Gaussian.draw(0, BAR_HEIGHT);
		main_rect.draw(Color(255, skel));
	}

	// album_list �`��
	{
		int32_t cou = first_cou;
		for (int32_t y = 0; y < (signed)z.height; ++y)
		{
			for (int32_t x = 0; x < (signed)z.width; ++x)
			{
				const Rect rect = MakeRect(x, y);
				if (!SelectImage(cou)) { break; }
				if (!rect.mouseOver)
				{
					rect(SelectImage(cou).resize(216, 216)).draw();
					rect.drawFrame(3, 0, Color(0, 0, 0));
					z[y][x] = Max(z[y][x] - 0.02, 0.0);
				}
				++cou;
			}
			if (!SelectImage(cou)) { break; }
		}
		cou = first_cou;
		for (int32_t y = 0; y < (signed)z.height; ++y)
		{
			for (int32_t x = 0; x < (signed)z.width; ++x)
			{
				const Rect rect = MakeRect(x, y);
				if (rect.mouseOver || z[y][x])
				{
					if (rect.mouseOver) { z[y][x] = Min(z[y][x] + 0.05, 0.5); }
				}
				const double s = z[y][x];
				if (!SelectImage(cou)) { break; }
				RectF(rect).stretched(s * 2).drawShadow({ 0,15 * s }, 32 * s, 10 * s);
				RectF(rect).stretched(s * 2)(SelectImage(cou).resize(216, 216)).draw();
				RectF(rect).stretched(s * 2).drawFrame(3, 0, Color(0, 0, 0));
				++cou;
			}
			if (!SelectImage(cou)) { break; }
		}
		cou = first_cou;
		for (int32_t y = 0; y < (signed)z.height; ++y)
		{
			for (int32_t x = 0; x < (signed)z.width; ++x)
			{
				const Rect rect = MakeRect(x, y);
				if (!SelectImage(cou)) { break; }
				if (rect.mouseOver)
				{
					comTime[cou].first = (comTime[cou].first == 0 ? Time::GetMillisec64() : comTime[cou].first);
					comTime[cou].second = Time::GetMillisec64();
					if (comTime[cou].second - comTime[cou].first >= COM_MESSAGE_MILLISEC) { DrawDetails(cou); }
				}
				else { comTime[cou].first = comTime[cou].second = 0; }
				++cou;
			}
			if (!SelectImage(cou)) { break; }
		}
	}
}

Rect MakeRect(int32_t x, int32_t y)
{
	return { 30 + x * 246,BAR_HEIGHT + 30 + y * 246,216,216 };
}

Texture SelectImage(int32_t cou)
{
	Texture res;

	// �A���o��
	if (cou < (signed)AlbumList.size()) { res = AlbumList[cou].image; }

	// ���C�ɓ���
	else if (cou == (signed)AlbumList.size()) { res = fav; }

	// �I��
	else if (cou == (signed)AlbumList.size() + 1) { res = power; }

	return res;
}

void DrawDetails(int32_t cou)
{
	const Point pos = Mouse::Pos();
	static Font font(16);
	auto name_width = font(AlbumList[cou].name).region();
	auto creator_width = font(AlbumList[cou].creator).region();
	auto width = Max(name_width.w, creator_width.w);
	if (cou % 3 == 0)
	{
		RoundRect(pos.x + 13, pos.y + 13, width + 27, 72, 27).drawShadow({ 0,15 }, 32, 10);
		RoundRect(pos.x + 13, pos.y + 13, width + 27, 72, 27).draw(Color({ 255,255,255 }, 120));
		RoundRect(pos.x + 13, pos.y + 13, width + 27, 72, 27).drawFrame(3);
		font(AlbumList[cou].name).draw(pos.x + 27, pos.y + 20, Color(16, 16, 16));
		font(AlbumList[cou].creator).draw(pos.x + 27, pos.y + 50, Color(16, 16, 16));
	}
	if (cou % 3 == 1)
	{
		RoundRect(pos.x - width / 2, pos.y + 13, width + 27, 72, 27).drawShadow({ 0,15 }, 32, 10);
		RoundRect(pos.x - width / 2, pos.y + 13, width + 27, 72, 27).draw(Color({ 255,255,255 }, 120));
		RoundRect(pos.x - width / 2, pos.y + 13, width + 27, 72, 27).drawFrame(3);
		font(AlbumList[cou].name).draw(pos.x - width / 2 + 14, pos.y + 20, Color(16, 16, 16));
		font(AlbumList[cou].creator).draw(pos.x - width / 2 + 14, pos.y + 50, Color(16, 16, 16));
	}
	if (cou % 3 == 2)
	{
		RoundRect(pos.x - width, pos.y + 13, width + 27, 72, 27).drawShadow({ 0,15 }, 32, 10);
		RoundRect(pos.x - width, pos.y + 13, width + 27, 72, 27).draw(Color({ 255,255,255 }, 120));
		RoundRect(pos.x - width, pos.y + 13, width + 27, 72, 27).drawFrame(3);
		font(AlbumList[cou].name).draw(pos.x - width + 14, pos.y + 20, Color(16, 16, 16));
		font(AlbumList[cou].creator).draw(pos.x - width + 14, pos.y + 50, Color(16, 16, 16));
	}
}