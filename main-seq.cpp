#include <string>
#include <iostream>
#include <cpr/cpr.h>
#include "gumbo.h"
#include <vector>
#include <string>
#include <algorithm>
#include "json.hpp"
#include <chrono>
#include <fstream>
typedef std::chrono::high_resolution_clock Time;

// for convenience
using json = nlohmann::json;
using namespace std;
bool found = false;
string URL = "https://www.kabum.com.br/computadores/computador-gamer";
bool found_next;
auto r = cpr::Get(cpr::Url{URL});

int counter = 0;
vector<string> url_list, product_names, product_descriptions,
    pic_urls, view_prices, financed_priced, product_categories, next_urls, all_urls;

vector<json> product_infos;

static void search_for_links(GumboNode *node)
{
    if (node->type != GUMBO_NODE_ELEMENT)
    {
        return;
    }
    GumboAttribute *href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href")))
    {

        if (href->value[25] == 'p')
        {
            if (find(url_list.begin(), url_list.end(), (href->value)) == url_list.end())
            {
                url_list.push_back(href->value);
                counter++;
            }
        }
    }

    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i)
    {
        search_for_links(static_cast<GumboNode *>(children->data[i]));
    }
}

static void find_urls(GumboNode *node)
{
    if (node->type == GUMBO_NODE_ELEMENT)
    {
        GumboAttribute *vAttribute;
        if (node->v.element.tag == GUMBO_TAG_DIV &&
            (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "id")))
        {

            // cout << vAttribute->value << endl << strcmp(vAttribute->value, "BlocoConteudo") << endl;

            if (strcmp(vAttribute->value, "BlocoConteudo") == 0)
            {
                GumboVector *children = &node->v.element.children;
                for (unsigned int i = 0; i < children->length; ++i)
                {
                    search_for_links(static_cast<GumboNode *>(children->data[i]));
                }
            }
        }

        GumboVector *children = &node->v.element.children;

        for (unsigned int i = 0; i < children->length; ++i)
        {
            find_urls(static_cast<GumboNode *>(children->data[i]));
        }
    }
}

static json find_product_infos(GumboNode *node, json j)
{
    if (node->type == GUMBO_NODE_ELEMENT)
    {
        GumboAttribute *vAttribute;

        //PRODUCT NAME
        if (node->v.element.tag == GUMBO_TAG_H1)
        {
            if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "itemprop"))
            {
                if (strcmp(vAttribute->value, "name") == 0)
                {
                    // product_names.push_back(vAttribute->value);

                    GumboNode *title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
                    if (title_text->type == GUMBO_NODE_TEXT)
                    {
                        j["name"] = title_text->v.text.text;
                        product_names.push_back(title_text->v.text.text);
                    }
                }
            }
        }

        //PRODUCT DESCRIPTION
        else if (node->v.element.tag == GUMBO_TAG_P)
        {
            if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "itemprop"))
            {
                if (strcmp(vAttribute->value, "description") == 0)
                {

                    GumboNode *title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
                    if (title_text->type == GUMBO_NODE_TEXT)
                    {
                        j["description"] = title_text->v.text.text;
                        product_descriptions.push_back(title_text->v.text.text);
                    }
                }
            }
        }

        //FINANCED PRICE
        else if (node->v.element.tag == GUMBO_TAG_DIV)
        {
            if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "class"))
            {
                if (strcmp(vAttribute->value, "preco_normal") == 0)
                {

                    GumboNode *title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
                    if (title_text->type == GUMBO_NODE_TEXT)
                    {
                        j["f_price"] = title_text->v.text.text;

                        financed_priced.push_back(title_text->v.text.text);
                    }
                }
            }
        }

        //VIEW PRICE
        else if (node->v.element.tag == GUMBO_TAG_SPAN)
        {
            if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "itemprop"))
            {
                if (strcmp(vAttribute->value, "offers") == 0)
                {
                    GumboVector *children = &node->v.element.children;
                    for (unsigned int i = 0; i < children->length; ++i)
                    {
                        GumboNode *node = static_cast<GumboNode *>(children->data[i]);
                        if (node->type == GUMBO_NODE_ELEMENT)
                        {
                            GumboAttribute *vAttribute;
                            if (node->v.element.tag == GUMBO_TAG_STRONG)
                            {
                                GumboNode *title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
                                if (title_text->type == GUMBO_NODE_TEXT)
                                {
                                    j["price"] = title_text->v.text.text;

                                    view_prices.push_back(title_text->v.text.text);
                                }
                            }
                        }
                    }
                }
            }
        }

        //PIC URL
        else if (node->v.element.tag == GUMBO_TAG_IMG)
        {
            if (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "itemprop"))
            {
                // cout << vAttribute->value << endl;
                if (strcmp(vAttribute->value, "image") == 0)
                {
                    GumboAttribute *url = gumbo_get_attribute(&node->v.element.attributes, "src");
                    string url_value = url->value;
                    if (!found)
                    {
                        j["pic_url"] = url_value;

                        found = true;
                    }
                }
            }
        }

        GumboVector *children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i)
        {
            j = find_product_infos(static_cast<GumboNode *>(children->data[i]), j);
        }
    }
    return j;
}

static void find_next_page(GumboNode *node)
{
    if (node->type == GUMBO_NODE_ELEMENT)
    {
        GumboAttribute *vAttribute;
        if (node->v.element.tag == GUMBO_TAG_A &&
            (vAttribute = gumbo_get_attribute(&node->v.element.attributes, "target")))
        {
            if (strcmp(vAttribute->value, "_top") == 0)
            {

                GumboNode *title_text = static_cast<GumboNode *>(node->v.element.children.data[0]);
                if (title_text->type == GUMBO_NODE_TEXT)
                {
                    if (strcmp(title_text->v.text.text, "Proxima > ") == 0)
                    {
                        vAttribute = gumbo_get_attribute(&node->v.element.attributes, "href");
                        if (!found_next)
                        {
                            next_urls.push_back(vAttribute->value);
                            found_next = true;
                        }
                    }
                }
            }
        }

        GumboVector *children = &node->v.element.children;

        for (unsigned int i = 0; i < children->length; ++i)
        {
            find_next_page(static_cast<GumboNode *>(children->data[i]));
        }
    }
}



static void create_jsons()
{

    GumboOutput *output = gumbo_parse(r.text.c_str());

    auto pos = URL.find_last_of('/');
    string category = URL.substr(pos + 1);
    string next_url;
    all_urls.push_back(URL);
    double download_time = 0;
    double process_time = 0;
    Time::time_point t1, t2;
    std::chrono::duration<double> diff;
    int n_prods = 0;

    while (!found_next)
    {

        t1 = Time::now();
        auto r = cpr::Get(cpr::Url{all_urls[all_urls.size() - 1]});
        t2 = Time::now();
        diff = t2-t1;
        download_time += diff.count();

        t1 = Time::now();
        output = gumbo_parse(r.text.c_str());
        find_urls(output->root);
        find_next_page(output->root);
        t2 = Time::now();
        diff = t2-t1;
        process_time += diff.count();
        
        for (int i = 0; i < url_list.size(); i++)
        {
            product_categories.push_back(category);
            n_prods++;

            t1 = Time::now();
            auto r = cpr::Get(cpr::Url{url_list[i]});
            t2 = Time::now();
            diff = t2-t1;
            download_time += diff.count();

            t1 = Time::now();
            GumboOutput *output = gumbo_parse(r.text.c_str());
            json product;
            product = find_product_infos(output->root, product);
            t2 = Time::now();
            diff = t2-t1;
            process_time += diff.count();
            cout << product << endl;

            found = false;
            // cout << '.';
            cout.flush();
        }
        url_list.clear();

        if (found_next)
        {
            found_next = false;
            next_url = URL + next_urls[next_urls.size() - 1];
            all_urls.push_back(next_url);

            t1 = Time::now();
            auto r2 = cpr::Get(cpr::Url{next_url});
            t2 = Time::now();
            diff = t2-t1;
            download_time += diff.count();

            t1 = Time::now();
            output = gumbo_parse(r2.text.c_str());
            t2 = Time::now();
            diff = t2-t1;
            process_time += diff.count();
        }
        else
        {
            found_next = true;
        }
        
    }
    gumbo_destroy_output(&kGumboDefaultOptions, output);

    cout << "TEMPO TOTAL DOWNLOAD: " << download_time << "s" <<endl;
    cout << "TOTAL DE PRODUTOS: " << n_prods <<endl;

    cout << "TEMPO TOTAL PROCESSAMENTO: " << process_time << "s" << endl;
}

int main(int argc, char **argv)
{

    double total_time;
    auto t1 = Time::now();
    string temp = argv[1];
    URL = temp;
    

    create_jsons();
    auto t2 = Time::now();;
    std::chrono::duration<double> diff = t2-t1;
    cout << "TEMPO TOTAL: " << diff.count() << "s" << endl;
}
